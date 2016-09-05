set str "Hello World"

puts [string length $str]

puts [string range abcd 2 end]

puts [string range abcd 1 end-1]

puts [string compare -nocase "Abc" "aBC"]

puts [string match a* alpha]
