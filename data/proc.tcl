proc hello { a } {
  puts "Hello $a"

  return 4

  puts "??"
}

set a [hello "World"]

puts $a

puts [info body hello]

puts [info args hello]

proc va { a args } {
  puts "$a [llength $args]"
}

va 1
va 1 2
va 1 {2 3} 4

proc embedded { } {
  hello "embedded"
}

embedded
