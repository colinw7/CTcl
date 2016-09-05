namespace eval random {
  variable seed [clock seconds]

  proc init { value } {
    variable seed
    set seed $value
  }

  proc random { } {
    variable seed
    set seed [expr {($seed*9301 + 49297) % 233280}]
    return [expr ($seed/double(233280))]
  }

  proc range { range } {
    expr {int([random]*$range)}
  }
}
