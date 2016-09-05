set l1 {1 2 3}
set l2 {1 2 3}

set l3 [linsert $l1 1 4]

set l4 [linsert $l1 2 $l2]

puts $l3
puts $l4
