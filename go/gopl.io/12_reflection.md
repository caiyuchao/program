## 12. Reflection
=================

#### 12.1 Why Reflection?
ometimes we need to write a function capable of dealing uniformly with values of types
that don’t satisfy a common interface, don’t have a known representation, or don’t exist at
the time we design the function—or even all three.

A familiar example is the formatting logic within fmt.Fprintf, which can usefully
print an arbitrary value of any type, even a user-defined one. Let’s try to implement a
function like it using what we know already. For simplicity, our function will accept one
argument and will return the result as a string like fmt.Sprint does, so we’ll call it
Sprint.

```go
func Sprint(x interface{}) string {
	type stringer interface {
		String() string
	}
	switch x := x.(type) {
		case stringer:
			return x.String()
		case string:
			return x
		case int:
			return strconv.Itoa(x)
		// ...similar cases for int16, uint32, and so on…
		case bool:
			if x {
				return "true"
			}
			return "false"
		default:
			// array, chan, func, map, pointer, slice, struct
			return "???"
	}
}
```
