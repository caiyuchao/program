package main

import (
	"time"

	"github.com/yubo/rrdlite"

	"github.com/open-falcon/model"
)

func create(filename string, item *model.GraphItem) error {
	now := time.Now()
	start := now.Add(time.Duration(-24) * time.Hour)
	step := uint(item.Step)

	c := rrdlite.NewCreator(filename, start, step)
	c.DS("metric", item.DsType, item.Heartbeat, item.Min, item.Max)

	// 设置各种归档策略
	// 1分钟一个点存 12小时
	c.RRA("AVERAGE", 0.5, 1, 720)

	// 5m一个点存2d
	c.RRA("AVERAGE", 0.5, 5, 576)
	c.RRA("MAX", 0.5, 5, 576)
	c.RRA("MIN", 0.5, 5, 576)

	// 20m一个点存7d
	c.RRA("AVERAGE", 0.5, 20, 504)
	c.RRA("MAX", 0.5, 20, 504)
	c.RRA("MIN", 0.5, 20, 504)

	// 3小时一个点存3个月
	c.RRA("AVERAGE", 0.5, 180, 766)
	c.RRA("MAX", 0.5, 180, 766)
	c.RRA("MIN", 0.5, 180, 766)

	// 1天一个点存5year
	c.RRA("AVERAGE", 0.5, 720, 730)
	c.RRA("MAX", 0.5, 720, 730)
	c.RRA("MIN", 0.5, 720, 730)

	return c.Create(true)
}

func main() {
	item := &model.GraphItem{
		DsType:    "GUAGE",
		Heartbeat: 120,
		Min:       "U",
		Max:       "U",
	}
	create("asdf.rrd", item)
}
