## Testing

#### 11.1 The go test Tool
The go test tool scans the *_test.go files for these special functions, generates a
temporary main package that calls them all in the proper way, builds and runs it, reports
the results, and then cleans up.


#### 11.2 Test Functions
Each test file must import the testing package. Test functions have the following signature:

```go
func TestName(t *testing.T) {
	// ...
}
```

```shell
$ cd $GOPATH/src/gopl.io/ch11/word1
$ go test
ok gopl.io/ch11/word1 0.008s
```

The -v flag prints the name and execution time of each test in the package:
```shell
$ go test -v
=== RUN TestPalindrome
--- PASS: TestPalindrome (0.00s)
=== RUN TestNonPalindrome
--- PASS: TestNonPalindrome (0.00s)
=== RUN TestFrenchPalindrome
--- FAIL: TestFrenchPalindrome (0.00s)
word_test.go:28: IsPalindrome("été") = false
=== RUN TestCanalPalindrome
--- FAIL: TestCanalPalindrome (0.00s)
word_test.go:35: IsPalindrome("A man, a plan, a canal: Panama") = false
FAIL
exit status 1
FAIL gopl.io/ch11/word1 0.017s
```
and the -run flag, whose argument is a regular expression,
causes go test to run only those tests whose function name
matches the pattern:
```shell
$ go test -v -run="French|Canal"
=== RUN TestFrenchPalindrome
--- FAIL: TestFrenchPalindrome (0.00s)
word_test.go:28: IsPalindrome("été") = false
=== RUN TestCanalPalindrome
--- FAIL: TestCanalPalindrome (0.00s)
word_test.go:35: IsPalindrome("A man, a plan, a canal: Panama") = false
FAIL
exit status 1
FAIL gopl.io/ch11/word1 0.014s
```

#### 11.2.1 Randomized Testing
```go
import "math/rand"
// randomPalindrome returns a palindrome whose length and contents
// are derived from the pseudo-random number generator rng.
func randomPalindrome(rng *rand.Rand) string {
	n := rng.Intn(25) // random length up to 24
	runes := make([]rune, n)
	for i := 0; i < (n+1)/2; i++ {
		r := rune(rng.Intn(0x1000)) // random rune up to '\u0999'
		runes[i] = r
		runes[n-1-i] = r
	}
	return string(runes)
} 
func TestRandomPalindromes(t *testing.T) {
	// Initialize a pseudo-random number generator.
	seed := time.Now().UTC().UnixNano()
	t.Logf("Random seed: %d", seed)
	rng := rand.New(rand.NewSource(seed))
	for i := 0; i < 1000; i++ {
		p := randomPalindrome(rng)
		if !IsPalindrome(p) {
			t.Errorf("IsPalindrome(%q) = false", p)
		}
	}
}
```

  - go list -json fmt
  - ch11/echo
  - ch11/storage1
  - ch11/storage2

#### 11.3 Coverage
  - go test -v -run=Coverage gopl.io/ch7/eval
  - go test -run=Coverage -coverprofile=c.out
  - go tool cover -html=c.out -o=coverage.html

#### 11.4 Benchmark Functions
```shell
$ cd $GOPATH/src/gopl.io/ch11/word2
$ go test -bench=.
PASS
BenchmarkIsPalindrome-8 1000000 1035 ns/op 
ok gopl.io/ch11/word2 2.179s
```

As this example shows, the fastest program is often the one that makes the fewest memory
allocations. The -benchmem command-line flag will include memory allocation statistics
in its report. Here we compare the number of allocations before the optimization:
```shell
$ go test -bench=. -benchmem
PASS
BenchmarkIsPalindrome 1000000 1026 ns/op 304 B/op 4 allocs/op
and after it:
$ go test -bench=. -benchmem
PASS
BenchmarkIsPalindrome 2000000 807 ns/op 128 B/op 1 allocs/op
```

#### 11.5 Profiling
```shell
$ go test -cpuprofile=cpu.out
$ go test -blockprofile=block.out
$ go test -memprofile=mem.out

$ go test -run=NONE -bench=ClientServerParallelTLS64 \
> -cpuprofile=cpu.log net/http
PASS
BenchmarkClientServerParallelTLS64-8         500           2870131 ns/op          270358 B/op       3275 allocs/op
ok      net/http        1.804s
$ go tool pprof -text -nodecount=10 ./http.test cpu.log
2120ms of 2820ms total (75.18%)
Dropped 90 nodes (cum <= 14.10ms)
Showing top 10 nodes out of 153 (cum >= 130ms)
      flat  flat%   sum%        cum   cum%
    1510ms 53.55% 53.55%     1520ms 53.90%  crypto/elliptic.p256ReduceDegree
     110ms  3.90% 57.45%      110ms  3.90%  syscall.Syscall
     100ms  3.55% 60.99%      110ms  3.90%  crypto/elliptic.p256Diff
      90ms  3.19% 64.18%      870ms 30.85%  crypto/elliptic.p256Mul
      80ms  2.84% 67.02%       80ms  2.84%  runtime.futex
      50ms  1.77% 68.79%       50ms  1.77%  crypto/elliptic.p256SelectJacobianPoint
      50ms  1.77% 70.57%       50ms  1.77%  crypto/sha256.block
      50ms  1.77% 72.34%       50ms  1.77%  runtime.heapBitsForObject
      40ms  1.42% 73.76%      780ms 27.66%  crypto/elliptic.p256Square
      40ms  1.42% 75.18%      130ms  4.61%  runtime.mallocgc
```

#### 11.6 Example Functions
The third kind of function treated specially by go test is an example function, one
whose name starts with Example. It has neither parameters nor results. Here’s an
example function for IsPalindrome:
```go
func ExampleIsPalindrome() {
	fmt.Println(IsPalindrome("A man, a plan, a canal: Panama"))
	fmt.Println(IsPalindrome("palindrome"))
	// Output:
	// true
	// false
}
```

An interactive example of strings.Join in godoc:
```go
//strings/example_test.go:ExampleJoin()
package strings_test
 
import (
    "fmt"
    "strings"
)
func ExampleJoin() {                                                                                                                                                        
    s := []string{"foo", "bar", "baz"}
    fmt.Println(strings.Join(s, ", "))
    // Output: foo, bar, baz
}

//godoc
package main

import (
	"fmt"
	"strings"
)

func main() {
	s := []string{"foo", "bar", "baz"}
	fmt.Println(strings.Join(s, ", "))
}
```
