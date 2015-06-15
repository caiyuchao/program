#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/mathematics.h"
#include "libavutil/avstring.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/dict.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"



AVDictionary *format_opts;

#define MAXBUFF  10240
typedef struct {
    char *data;
    int size;
    int l;
}probe_data;

static probe_data gdata={NULL,0,0};
static int do_show_error   = 0;
static int do_show_format  = 1;
static int do_show_frames  = 0;
static int do_show_packets = 0;
static int do_show_streams = 1;
static int do_show_program_version  = 0;
static int do_show_library_versions = 0;

static int show_value_unit              = 0;
static int use_value_prefix             = 0;
static int use_byte_value_binary_prefix = 0;
static int use_value_sexagesimal_format = 0;
static int show_private_data            = 1;

static char *print_format;



/* FFprobe context */
static const char *input_filename;
static AVInputFormat *iformat = NULL;

static const char *binary_unit_prefixes [] = { "", "Ki", "Mi", "Gi", "Ti", "Pi" };
static const char *decimal_unit_prefixes[] = { "", "K" , "M" , "G" , "T" , "P"  };

static const char *unit_second_str          = "s"    ;
static const char *unit_hertz_str           = "Hz"   ;
static const char *unit_byte_str            = "byte" ;
static const char *unit_bit_per_second_str  = "bit/s";


static void dLog(const char* Msg, ...)
{
        va_list Args;
        char s[MAXBUFF]={0};
        va_start(Args,Msg);
        vsnprintf(s+strlen(s),sizeof(s)-strlen(s), Msg, Args);
        va_end(Args);
        printf("%s\n",s);
}

static void gprintf(const char* Msg, ...)
{
        va_list Args;
        //char s[1024]={0};
        va_start(Args,Msg);
        //snprintf(s+strlen(s),sizeof(s)-strlen(s), Msg, Args);
        gdata.l = vsnprintf(gdata.data,gdata.size, Msg, Args);
        va_end(Args);
        dLog("get_probe:gprintf:data[%s] size[%d] add[%d]",gdata.data, gdata.size, gdata.l);
        gdata.data += gdata.l;
        gdata.size -= gdata.l; 
}



//__YUBO


struct unit_value {
    union { double d; long long int i; } val;
    const char *unit;
};

static char *value_string(char *buf, int buf_size, struct unit_value uv)
{
    double vald;
    int show_float = 0;

    if (uv.unit == unit_second_str) {
        vald = uv.val.d;
        show_float = 1;
    } else {
        vald = uv.val.i;
    }

    if (uv.unit == unit_second_str && use_value_sexagesimal_format) {
        double secs;
        int hours, mins;
        secs  = vald;
        mins  = (int)secs / 60;
        secs  = secs - mins * 60;
        hours = mins / 60;
        mins %= 60;
        snprintf(buf, buf_size, "%d:%02d:%09.6f", hours, mins, secs);
    } else {
        const char *prefix_string = "";
        int l;

        if (use_value_prefix && vald > 1) {
            long long int index;

            if (uv.unit == unit_byte_str && use_byte_value_binary_prefix) {
                index = (long long int) (log(vald)/log(2)) / 10;
                index = av_clip(index, 0, FF_ARRAY_ELEMS(binary_unit_prefixes) - 1);
                vald /= pow(2, index * 10);
                prefix_string = binary_unit_prefixes[index];
            } else {
                index = (long long int) (log10(vald)) / 3;
                index = av_clip(index, 0, FF_ARRAY_ELEMS(decimal_unit_prefixes) - 1);
                vald /= pow(10, index * 3);
                prefix_string = decimal_unit_prefixes[index];
            }
        }

        if (show_float || (use_value_prefix && vald != (long long int)vald))
            l = snprintf(buf, buf_size, "%f", vald);
        else
            l = snprintf(buf, buf_size, "%lld", (long long int)vald);
        snprintf(buf+l, buf_size-l, "%s%s%s", *prefix_string || show_value_unit ? " " : "",
                 prefix_string, show_value_unit ? uv.unit : "");
    }

    return buf;
}


/* WRITERS API */

typedef struct WriterContext WriterContext;

#define WRITER_FLAG_DISPLAY_OPTIONAL_FIELDS 1
#define WRITER_FLAG_PUT_PACKETS_AND_FRAMES_IN_SAME_CHAPTER 2

typedef struct Writer {
    int priv_size;                  ///< private size for the writer context
    const char *name;

    int  (*init)  (WriterContext *wctx, const char *args, void *opaque);
    void (*uninit)(WriterContext *wctx);

    void (*print_header)(WriterContext *ctx);
    void (*print_footer)(WriterContext *ctx);

    void (*print_chapter_header)(WriterContext *wctx, const char *);
    void (*print_chapter_footer)(WriterContext *wctx, const char *);
    void (*print_section_header)(WriterContext *wctx, const char *);
    void (*print_section_footer)(WriterContext *wctx, const char *);
    void (*print_integer)       (WriterContext *wctx, const char *, long long int);
    void (*print_string)        (WriterContext *wctx, const char *, const char *);
    void (*show_tags)           (WriterContext *wctx, AVDictionary *dict);
    int flags;                  ///< a combination or WRITER_FLAG_*
} Writer;

struct WriterContext {
    const AVClass *class;           ///< class of the writer
    const Writer *writer;           ///< the Writer of which this is an instance
    char *name;                     ///< name of this writer instance
    void *priv;                     ///< private data for use by the filter
    unsigned int nb_item;           ///< number of the item printed in the given section, starting at 0
    unsigned int nb_section;        ///< number of the section printed in the given section sequence, starting at 0
    unsigned int nb_chapter;        ///< number of the chapter, starting at 0
};

static const char *writer_get_name(void *p)
{
    WriterContext *wctx = p;
    return wctx->writer->name;
}

static const AVClass writer_class = {
    "Writer",
    writer_get_name,
    NULL,
    LIBAVUTIL_VERSION_INT,
};

static void writer_close(WriterContext **wctx)
{
    if (!*wctx)
        return;

    if ((*wctx)->writer->uninit)
        (*wctx)->writer->uninit(*wctx);
    av_freep(&((*wctx)->priv));
    av_freep(wctx);
}

static int writer_open(WriterContext **wctx, const Writer *writer,
                       const char *args, void *opaque)
{
    int ret = 0;

    if (!(*wctx = av_malloc(sizeof(WriterContext)))) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    if (!((*wctx)->priv = av_mallocz(writer->priv_size))) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    (*wctx)->class = &writer_class;

    
    (*wctx)->writer = writer;
    if ((*wctx)->writer->init){
        ret = (*wctx)->writer->init(*wctx, args, opaque);
        dLog("get_probe:writer_open:(*wctx)->writer->init() ret[%d]", ret); 
    }
    if (ret < 0)
        goto fail;

    return 0;

fail:
    writer_close(wctx);
    return ret;
}

static inline void writer_print_header(WriterContext *wctx)
{
    if (wctx->writer->print_header)
        wctx->writer->print_header(wctx);
    wctx->nb_chapter = 0;
}


static inline void writer_print_footer(WriterContext *wctx)
{
    if (wctx->writer->print_footer)
        wctx->writer->print_footer(wctx);
}

static inline void writer_print_chapter_header(WriterContext *wctx,
                                               const char *chapter)
{
    if (wctx->writer->print_chapter_header)
        wctx->writer->print_chapter_header(wctx, chapter);
    wctx->nb_section = 0;
}

static inline void writer_print_chapter_footer(WriterContext *wctx,
                                               const char *chapter)
{
    if (wctx->writer->print_chapter_footer)
        wctx->writer->print_chapter_footer(wctx, chapter);
    wctx->nb_chapter++;
}

static inline void writer_print_section_header(WriterContext *wctx,
                                               const char *section)
{
    if (wctx->writer->print_section_header)
        wctx->writer->print_section_header(wctx, section);
    wctx->nb_item = 0;
}

static inline void writer_print_section_footer(WriterContext *wctx,
                                               const char *section)
{
    if (wctx->writer->print_section_footer)
        wctx->writer->print_section_footer(wctx, section);
    wctx->nb_section++;
}

static inline void writer_print_integer(WriterContext *wctx,
                                        const char *key, long long int val)
{
    wctx->writer->print_integer(wctx, key, val);
    wctx->nb_item++;
}

static inline void writer_print_string(WriterContext *wctx,
                                       const char *key, const char *val, int opt)
{
    if (opt && !(wctx->writer->flags & WRITER_FLAG_DISPLAY_OPTIONAL_FIELDS))
        return;
    wctx->writer->print_string(wctx, key, val);
    wctx->nb_item++;
}

static void writer_print_time(WriterContext *wctx, const char *key,
                              int64_t ts, const AVRational *time_base)
{
    char buf[128];

    if (ts == AV_NOPTS_VALUE) {
        writer_print_string(wctx, key, "N/A", 1);
    } else {
        double d = ts * av_q2d(*time_base);
        value_string(buf, sizeof(buf), (struct unit_value){.val.d=d, .unit=unit_second_str});
        writer_print_string(wctx, key, buf, 0);
    }
}

static void writer_print_ts(WriterContext *wctx, const char *key, int64_t ts)
{
    if (ts == AV_NOPTS_VALUE) {
        writer_print_string(wctx, key, "N/A", 1);
    } else {
        writer_print_integer(wctx, key, ts);
    }
}

static inline void writer_show_tags(WriterContext *wctx, AVDictionary *dict)
{
    wctx->writer->show_tags(wctx, dict);
}

#define MAX_REGISTERED_WRITERS_NB 64

static const Writer *registered_writers[MAX_REGISTERED_WRITERS_NB + 1];

static int writer_register(const Writer *writer)
{
    static int next_registered_writer_idx = 0;

    if (next_registered_writer_idx == MAX_REGISTERED_WRITERS_NB)
        return AVERROR(ENOMEM);

    registered_writers[next_registered_writer_idx++] = writer;
    return 0;
}

static const Writer *writer_get_by_name(const char *name)
{
    int i;

    for (i = 0; registered_writers[i]; i++)
        if (!strcmp(registered_writers[i]->name, name))
            return registered_writers[i];

    return NULL;
}


#define ESCAPE_INIT_BUF_SIZE 256

#define ESCAPE_CHECK_SIZE(src, size, max_size)                          \
    if (size > max_size) {                                              \
        char buf[64];                                                   \
        snprintf(buf, sizeof(buf), "%s", src);                          \
        dLog(                                 \
               "String '%s...' with is too big\n", buf);                \
        return "FFPROBE_TOO_BIG_STRING";                                \
    }

#define ESCAPE_REALLOC_BUF(dst_size_p, dst_p, src, size)                \
    if (*dst_size_p < size) {                                           \
        char *q = av_realloc(*dst_p, size);                             \
        if (!q) {                                                       \
            char buf[64];                                               \
            snprintf(buf, sizeof(buf), "%s", src);                      \
            dLog(                             \
                   "String '%s...' could not be escaped\n", buf);       \
            return "FFPROBE_THIS_STRING_COULD_NOT_BE_ESCAPED";          \
        }                                                               \
        *dst_size_p = size;                                             \
        *dst = q;                                                       \
    }
    

/* JSON output */

typedef struct {
    const AVClass *class;
    int multiple_entries; ///< tells if the given chapter requires multiple entries
    char *buf;
    size_t buf_size;
    int print_packets_and_frames;
    int indent_level;
    int compact;
    const char *item_sep, *item_start_end;
} JSONContext;

#undef OFFSET
#define OFFSET(x) offsetof(JSONContext, x)
static const AVOption json_options[]= {
    { "compact", "enable compact output", OFFSET(compact), AV_OPT_TYPE_INT, {.dbl=0}, 0, 1 },
    { "c",       "enable compact output", OFFSET(compact), AV_OPT_TYPE_INT, {.dbl=0}, 0, 1 },
    { NULL }
};

static const char *json_get_name(void *ctx)
{
    return "json";
}

static const AVClass json_class = {
    "JSONContext",
    json_get_name,
    json_options
};

static av_cold int json_init(WriterContext *wctx, const char *args, void *opaque)
{
    JSONContext *json = wctx->priv;
    int err;


    dLog("get_probe:json_init begin"); 

    json->class = &json_class;
    av_opt_set_defaults(json);
    json->compact = 1;


//	    if (args &&
//	        (err = (av_set_options_string(json, args, "=", ":"))) < 0) {
//	        av_log(wctx, AV_LOG_ERROR, "Error parsing options string: '%s'\n", args);
//	        return err;
//	    }



    json->item_sep       = json->compact ? ", " : ",\n";
    json->item_start_end = json->compact ? " "  : "\n";

    json->buf_size = ESCAPE_INIT_BUF_SIZE;
    if (!(json->buf = av_malloc(json->buf_size)))
        return AVERROR(ENOMEM);

    return 0;
}

static av_cold void json_uninit(WriterContext *wctx)
{
    JSONContext *json = wctx->priv;
    av_freep(&json->buf);
}

static const char *json_escape_str(char **dst, size_t *dst_size, const char *src,
                                   void *log_ctx)
{
    static const char json_escape[] = {'"', '\\', '\b', '\f', '\n', '\r', '\t', 0};
    static const char json_subst[]  = {'"', '\\',  'b',  'f',  'n',  'r',  't', 0};
    const char *p;
    char *q;
    size_t size = 1;

    // compute the length of the escaped string
    for (p = src; *p; p++) {
        ESCAPE_CHECK_SIZE(src, size, SIZE_MAX-6);
        if (strchr(json_escape, *p))     size += 2; // simple escape
        else if ((unsigned char)*p < 32) size += 6; // handle non-printable chars
        else                             size += 1; // char copy
    }
    ESCAPE_REALLOC_BUF(dst_size, dst, src, size);

    q = *dst;
    for (p = src; *p; p++) {
        char *s = strchr(json_escape, *p);
        if (s) {
            *q++ = '\\';
            *q++ = json_subst[s - json_escape];
        } else if ((unsigned char)*p < 32) {
            snprintf(q, 7, "\\u00%02x", *p & 0xff);
            q += 6;
        } else {
            *q++ = *p;
        }
    }
    *q = 0;
    return *dst;
}


static void json_print_header(WriterContext *wctx)
{
    JSONContext *json = wctx->priv;
    //printf("{");
    gprintf("{");  
    json->indent_level++;
}

static void json_print_footer(WriterContext *wctx)
{
    JSONContext *json = wctx->priv;
    json->indent_level--;
    //printf("\n}\n");
    gprintf( "}");
}

#define JSON_INDENT()  gprintf("%*c", json->indent_level * 4, ' ')


static void json_print_chapter_header(WriterContext *wctx, const char *chapter)
{
    JSONContext *json = wctx->priv;

    if (wctx->nb_chapter)
        gprintf(",");
    //gprintf("\n");
    json->multiple_entries = !strcmp(chapter, "packets") || !strcmp(chapter, "frames" ) ||
                             !strcmp(chapter, "packets_and_frames") ||
                             !strcmp(chapter, "streams") || !strcmp(chapter, "library_versions");
    if (json->multiple_entries) {
        JSON_INDENT();
        gprintf( "\"%s\": [", json_escape_str(&json->buf, &json->buf_size, chapter, wctx));
        //printf("\"%s\": [\n", json_escape_str(&json->buf, &json->buf_size, chapter, wctx));
        json->print_packets_and_frames = !strcmp(chapter, "packets_and_frames");
        json->indent_level++;
    }
}

static void json_print_chapter_footer(WriterContext *wctx, const char *chapter)
{
    JSONContext *json = wctx->priv;

    if (json->multiple_entries) {
        //gprintf("\n");
        json->indent_level--;
        JSON_INDENT();
        gprintf("]");
    }
}

static void json_print_section_header(WriterContext *wctx, const char *section)
{
    JSONContext *json = wctx->priv;

    if (wctx->nb_section)
        gprintf(","); //gprintf(",\n");
    JSON_INDENT();
    if (!json->multiple_entries){
        gprintf("\"%s\": ", section);
    }
    gprintf("{%s", json->item_start_end);
    json->indent_level++;
    /* this is required so the parser can distinguish between packets and frames */
    if (json->print_packets_and_frames) {
        if (!json->compact)
            JSON_INDENT();
        gprintf("\"type\": \"%s\"%s", section, json->item_sep);
    }
}

static void json_print_section_footer(WriterContext *wctx, const char *section)
{
    JSONContext *json = wctx->priv;

    gprintf("%s", json->item_start_end);
    json->indent_level--;
    if (!json->compact)
        JSON_INDENT();
    gprintf("}");
}

static inline void json_print_item_str(WriterContext *wctx,
                                       const char *key, const char *value)
{
    JSONContext *json = wctx->priv;

    dLog("get_probe:json_print_item_str key[%s] value[%s]", key ,value);
    gprintf("\"%s\":", json_escape_str(&json->buf, &json->buf_size, key,   wctx));
    gprintf(" \"%s\"", json_escape_str(&json->buf, &json->buf_size, value, wctx));
}

static void json_print_str(WriterContext *wctx, const char *key, const char *value)
{
    JSONContext *json = wctx->priv;

    if (wctx->nb_item) gprintf("%s", json->item_sep);
    if (!json->compact)
        JSON_INDENT();
    json_print_item_str(wctx, key, value);
}

static void json_print_int(WriterContext *wctx, const char *key, long long int value)
{
    JSONContext *json = wctx->priv;

    if (wctx->nb_item) gprintf("%s", json->item_sep);
    if (!json->compact)
        JSON_INDENT();
    gprintf("\"%s\": %lld",
           json_escape_str(&json->buf, &json->buf_size, key, wctx), value);
}

static void json_show_tags(WriterContext *wctx, AVDictionary *dict)
{
    JSONContext *json = wctx->priv;
    AVDictionaryEntry *tag = NULL;
    int is_first = 1;
    if (!dict)
        return;
    gprintf("%s", json->item_sep);
    if (!json->compact)
        JSON_INDENT();
    gprintf("\"tags\": {%s", json->item_start_end);
    json->indent_level++;
    while ((tag = av_dict_get(dict, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        if (is_first) is_first = 0;
        else          gprintf("%s", json->item_sep);
        if (!json->compact)
            JSON_INDENT();
        json_print_item_str(wctx, tag->key, tag->value);
    }
    json->indent_level--;
    gprintf("%s", json->item_start_end);
    if (!json->compact)
        JSON_INDENT();
    gprintf("}");
}

static const Writer json_writer = {
    .name                 = "json",
    .priv_size            = sizeof(JSONContext),
    .init                 = json_init,
    .uninit               = json_uninit,
    .print_header         = json_print_header,
    .print_footer         = json_print_footer,
    .print_chapter_header = json_print_chapter_header,
    .print_chapter_footer = json_print_chapter_footer,
    .print_section_header = json_print_section_header,
    .print_section_footer = json_print_section_footer,
    .print_integer        = json_print_int,
    .print_string         = json_print_str,
    .show_tags            = json_show_tags,
    .flags = WRITER_FLAG_PUT_PACKETS_AND_FRAMES_IN_SAME_CHAPTER,
};

struct print_buf {
    char *s;
    int len;
};

static char *fast_asprintf(struct print_buf *pbuf, const char *fmt, ...)
{
    va_list va;
    int len;

    va_start(va, fmt);
    len = vsnprintf(NULL, 0, fmt, va);
    va_end(va);
    if (len < 0)
        goto fail;

    if (pbuf->len < len) {
        char *p = av_realloc(pbuf->s, len + 1);
        if (!p)
            goto fail;
        pbuf->s   = p;
        pbuf->len = len;
    }

    va_start(va, fmt);
    len = vsnprintf(pbuf->s, len + 1, fmt, va);
    va_end(va);
    if (len < 0)
        goto fail;
    return pbuf->s;

fail:
    av_freep(&pbuf->s);
    pbuf->len = 0;
    return NULL;
}

static void writer_register_all(void)
{
    static int initialized;

    if (initialized)
        return;
    initialized = 1;
    writer_register(&json_writer);
}


#define print_fmt(k, f, ...) do {              \
    if (fast_asprintf(&pbuf, f, __VA_ARGS__))  \
        writer_print_string(w, k, pbuf.s, 0);  \
} while (0)

#define print_fmt_opt(k, f, ...) do {          \
    if (fast_asprintf(&pbuf, f, __VA_ARGS__))  \
        writer_print_string(w, k, pbuf.s, 1);  \
} while (0)

#define print_int(k, v)         writer_print_integer(w, k, v)
#define print_str(k, v)         writer_print_string(w, k, v, 0)
#define print_str_opt(k, v)     writer_print_string(w, k, v, 1)
#define print_time(k, v, tb)    writer_print_time(w, k, v, tb)
#define print_ts(k, v)          writer_print_ts(w, k, v)
#define print_val(k, v, u)      writer_print_string(w, k, \
    value_string(val_str, sizeof(val_str), (struct unit_value){.val.i = v, .unit=u}), 0)
#define print_section_header(s) writer_print_section_header(w, s)
#define print_section_footer(s) writer_print_section_footer(w, s)
#define show_tags(metadata)     writer_show_tags(w, metadata)



static void show_stream(WriterContext *w, AVFormatContext *fmt_ctx, int stream_idx)
{
    AVStream *stream = fmt_ctx->streams[stream_idx];
    AVCodecContext *dec_ctx;
    AVCodec *dec;
    char val_str[128];
    const char *s;
    AVRational display_aspect_ratio;
    struct print_buf pbuf = {.s = NULL};

    print_section_header("stream");

    print_int("index", stream->index);

    if ((dec_ctx = stream->codec)) {
        if ((dec = dec_ctx->codec)) {
            print_str("codec_name",      dec->name);
            print_str("codec_long_name", dec->long_name);
        } else {
            print_str_opt("codec_name",      "unknown");
            print_str_opt("codec_long_name", "unknown");
        }

        s = av_get_media_type_string(dec_ctx->codec_type);
        if (s) print_str    ("codec_type", s);
        else   print_str_opt("codec_type", "unknown");
        print_fmt("codec_time_base", "%d/%d", dec_ctx->time_base.num, dec_ctx->time_base.den);

        /* print AVI/FourCC tag */
        av_get_codec_tag_string(val_str, sizeof(val_str), dec_ctx->codec_tag);
        print_str("codec_tag_string",    val_str);
        print_fmt("codec_tag", "0x%04x", dec_ctx->codec_tag);

        switch (dec_ctx->codec_type) {
        case AVMEDIA_TYPE_VIDEO:
            print_int("width",        dec_ctx->width);
            print_int("height",       dec_ctx->height);
            print_int("has_b_frames", dec_ctx->has_b_frames);
            if (dec_ctx->sample_aspect_ratio.num) {
                print_fmt("sample_aspect_ratio", "%d:%d",
                          dec_ctx->sample_aspect_ratio.num,
                          dec_ctx->sample_aspect_ratio.den);
                av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
                          dec_ctx->width  * dec_ctx->sample_aspect_ratio.num,
                          dec_ctx->height * dec_ctx->sample_aspect_ratio.den,
                          1024*1024);
                print_fmt("display_aspect_ratio", "%d:%d",
                          display_aspect_ratio.num,
                          display_aspect_ratio.den);
            } else {
                print_str_opt("sample_aspect_ratio", "N/A");
                print_str_opt("display_aspect_ratio", "N/A");
            }
            s = av_get_pix_fmt_name(dec_ctx->pix_fmt);
            if (s) print_str    ("pix_fmt", s);
            else   print_str_opt("pix_fmt", "unknown");
            print_int("level",   dec_ctx->level);
            if (dec_ctx->timecode_frame_start >= 0) {
                uint32_t tc = dec_ctx->timecode_frame_start;
                print_fmt("timecode", "%02d:%02d:%02d%c%02d",
                          tc>>19 & 0x1f,              // hours
                          tc>>13 & 0x3f,              // minutes
                          tc>>6  & 0x3f,              // seconds
                          tc     & 1<<24 ? ';' : ':', // drop
                          tc     & 0x3f);             // frames
            } else {
                print_str_opt("timecode", "N/A");
            }
            break;

        case AVMEDIA_TYPE_AUDIO:
            s = av_get_sample_fmt_name(dec_ctx->sample_fmt);
            if (s) print_str    ("sample_fmt", s);
            else   print_str_opt("sample_fmt", "unknown");
            print_val("sample_rate",     dec_ctx->sample_rate, unit_hertz_str);
            print_int("channels",        dec_ctx->channels);
            print_int("bits_per_sample", av_get_bits_per_sample(dec_ctx->codec_id));
            break;
        }
    } else {
        print_str_opt("codec_type", "unknown");
    }
    if (dec_ctx->codec && dec_ctx->codec->priv_class && show_private_data) {
        const AVOption *opt = NULL;
        while (opt = av_opt_next(dec_ctx->priv_data,opt)) {
            uint8_t *str;
            if (opt->flags) continue;
            if (av_opt_get(dec_ctx->priv_data, opt->name, 0, &str) >= 0) {
                print_str(opt->name, str);
                av_free(str);
            }
        }
    }

    if (fmt_ctx->iformat->flags & AVFMT_SHOW_IDS) print_fmt    ("id", "0x%x", stream->id);
    else                                          print_str_opt("id", "N/A");
    print_fmt("r_frame_rate",   "%d/%d", stream->r_frame_rate.num,   stream->r_frame_rate.den);
    print_fmt("avg_frame_rate", "%d/%d", stream->avg_frame_rate.num, stream->avg_frame_rate.den);
    print_fmt("time_base",      "%d/%d", stream->time_base.num,      stream->time_base.den);
    print_time("start_time",    stream->start_time, &stream->time_base);
    print_time("duration",      stream->duration,   &stream->time_base);
    if (stream->nb_frames) print_fmt    ("nb_frames", "%"PRId64, stream->nb_frames);
    else                   print_str_opt("nb_frames", "N/A");
    show_tags(stream->metadata);

    print_section_footer("stream");
    av_free(pbuf.s);
    fflush(stdout);
}

static void show_streams(WriterContext *w, AVFormatContext *fmt_ctx)
{
    int i;
    for (i = 0; i < fmt_ctx->nb_streams; i++)
        show_stream(w, fmt_ctx, i);
}

static void show_format(WriterContext *w, AVFormatContext *fmt_ctx)
{
    char val_str[128];
    int64_t size = fmt_ctx->pb ? avio_size(fmt_ctx->pb) : -1;

    print_section_header("format");
    print_str("filename",         fmt_ctx->filename);
    print_int("nb_streams",       fmt_ctx->nb_streams);
    print_str("format_name",      fmt_ctx->iformat->name);
    //print_str("format_long_name", fmt_ctx->iformat->long_name);
    print_time("start_time",      fmt_ctx->start_time, &AV_TIME_BASE_Q);
    print_time("duration",        fmt_ctx->duration,   &AV_TIME_BASE_Q);
    if (size >= 0) print_val    ("size", size, unit_byte_str);
    else           print_str_opt("size", "N/A");
    if (fmt_ctx->bit_rate > 0) print_val    ("bit_rate", fmt_ctx->bit_rate, unit_bit_per_second_str);
    else                       print_str_opt("bit_rate", "N/A");
    show_tags(fmt_ctx->metadata);
    print_section_footer("format");
    fflush(stdout);
}

static void show_error(WriterContext *w, int err)
{
    char errbuf[128];
    const char *errbuf_ptr = errbuf;

    if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
        errbuf_ptr = strerror(AVUNERROR(err));

    writer_print_chapter_header(w, "error");
    print_section_header("error");
    print_int("code", err);
    print_str("string", errbuf_ptr);
    print_section_footer("error");
    writer_print_chapter_footer(w, "error");
}

static int open_input_file(AVFormatContext **fmt_ctx_ptr, const char *filename)
{
    int err, i;
    AVFormatContext *fmt_ctx = NULL;
    AVDictionaryEntry *t;

    if ((err = avformat_open_input(&fmt_ctx, filename,
                                   iformat, &format_opts)) < 0) {
        return err;
    }
    if ((t = av_dict_get(format_opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
        dLog("get_probe:Option %s not found.\n", t->key);
        return AVERROR_OPTION_NOT_FOUND;
    }


    /* fill the streams in the format context */
    if ((err = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        return err;
    }

    //av_dump_format(fmt_ctx, 0, filename, 0);

    /* bind a decoder to each input stream */
    for (i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream *stream = fmt_ctx->streams[i];
        AVCodec *codec;

        if (!(codec = avcodec_find_decoder(stream->codec->codec_id))) {
            dLog(
                    "get_probe:Unsupported codec with id %d for input stream %d\n",
                    stream->codec->codec_id, stream->index);
        } else if (avcodec_open2(stream->codec, codec, NULL) < 0) {
            dLog("get_probe:Error while opening codec for input stream %d\n",
                   stream->index);
        }
    }

    *fmt_ctx_ptr = fmt_ctx;
    return 0;
}

#define PRINT_CHAPTER(name) do {                                        \
    if (do_show_ ## name) {                                             \
        writer_print_chapter_header(wctx, #name);                       \
        show_ ## name (wctx, fmt_ctx);                                  \
        writer_print_chapter_footer(wctx, #name);                       \
    }                                                                   \
} while (0)

static int probe_file(WriterContext *wctx, const char *filename)
{
    AVFormatContext *fmt_ctx;
    int ret, i;

    ret = open_input_file(&fmt_ctx, filename);
    if (ret >= 0) {
        PRINT_CHAPTER(streams);
        PRINT_CHAPTER(format);
        for (i = 0; i < fmt_ctx->nb_streams; i++)
            if (fmt_ctx->streams[i]->codec->codec_id != CODEC_ID_NONE)
                avcodec_close(fmt_ctx->streams[i]->codec);
        avformat_close_input(&fmt_ctx);
    }

    return ret;
}


//__YUBO


int get_probe( const char* filename, char* info){
    const Writer *w;
    WriterContext *wctx;
    int ret;
    //char path[256];

    //strcpy(path, filename);
    //decode_filename(filename,input_filename);
    dLog("get_probe:filename is [%s]",filename); 



    writer_register_all();
    w = writer_get_by_name("json");
    dLog("get_probe:writer_get_by_name is [0x%x]",w); 
    

    if ((ret = writer_open(&wctx, w, NULL, NULL)) >= 0) {
        dLog("get_probe:writer_open is ok"); 

        gdata.data= info;
        gdata.size = MAXBUFF;
        
        writer_print_header(wctx);
        


        if (filename) {
            ret = probe_file(wctx, filename);
            dLog("get_probe:probe_file ret[%d]",ret); 
        }

        writer_print_footer(wctx);
        writer_close(&wctx);
    }

    return ret;
}


int IsSpace(int c)
{
    return c==' ' || c=='\n' || c=='\t';
}

void LTrim(char *s) 
{
       char *p; 

        for (p=s;IsSpace(*p);p++);
        while (*p)
           *(s++) = *(p++);
        *s=0;
}

int ffmpeg(char *cmd);

int main(){
    int ret,argc;
    int width=120;
    int height=90;
    char info[MAXBUFF];
    char *inputfile = "/root/1.3gp";
    char *outputfile = "/root/1.jpg";
	int  starttime = 10;
	char cmd[1024];


	sprintf(cmd,"ffmpeg -i %s -y -vframes 1 -ss %d -s %d*%d %s", inputfile, starttime, width, height, outputfile);

    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    
    ret = get_probe(inputfile,info);
    if (ret) return ret; 
	ret = ffmpeg(cmd);
    
    dLog("info is [%s]",info);
}
