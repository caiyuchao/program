## packages and the go tool

#### 10.7.3 Building Packages

```shell
$ go build gopl.io/ch10/cross
$ ./cross
darwin amd64
$ GOARCH=386 go build gopl.io/ch10/cross
$ ./cross
darwin 386
```

#### 10.7.4 Documenting Packages
```shell
$ go doc time
$ go doc time.Since
$ go doc time.Duration.Seconds
$ go doc json.decode
```
