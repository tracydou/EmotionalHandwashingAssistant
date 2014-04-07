language: PYTHON
name:     "dtree"

# floats
variable {
 name: "GAIN"
 type: FLOAT
 size: 1
 min:  0.0
 max:  0.5
}

# integers

variable {
 name: "OFFSET"
 type: INT
 size: 1
 min:  10
 max:  250
}

variable {
 name: "DEPTH"
 type: INT
 size: 1
 min:  4
 max:  20
}
 
#variable {
# name: "THRESHOLD"
# type: INT
# size: 1
# min:  10
# max:  150
#}

# Enumeration example
# 
# variable {
#  name: "Z"
#  type: ENUM
#  size: 3
#  options: "foo"
#  options: "bar"
#  options: "baz"
# }


