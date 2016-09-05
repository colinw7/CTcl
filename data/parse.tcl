set a 1
set b {1 2 3}
set c {{1 2} {3 4}}

puts "hello"
puts $a.a
puts $b.b
puts $c.c

puts $a[pwd]
puts [pwd]$a

foreach x {a b c} {
  puts $a.$x
  puts $b.$x
  puts $c.$x

  puts $x[pwd]
  puts [pwd]$x
}
