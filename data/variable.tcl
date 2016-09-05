set a 1
set b 2

puts $a
puts $b

namespace eval temp {
  variable a

  set a 2
  set b 3

  puts $a
  puts $b
}

puts $a
puts $b
