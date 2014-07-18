package main

func buildSweep() [][]int {
	iperms := Permute(skipsums, 11, 11, 11, 11, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, 2, 2, 2, 1)
	perms := [][]int{}
	for _, p := range iperms {
		perms = append(perms, p)
	}
	return perms
}

func skipsums(p []int) bool {
	if Sum(p, 0, 5) > 10 {
		return true
	} else if Sum(p, 5, 10) > 1 {
		return true
	} else if Sum(p, 10, 15) > 1 {
		return true
	} else if Sum(p, 15, 20) > 1 {
		return true
	}
	return false
}

func Sum(s []int, lower, upper int) int {
	if upper > len(s) {
		upper = len(s)
	}
	if lower > len(s) {
		lower = len(s)
	}

	tot := 0
	for _, v := range s[lower:upper] {
		tot += v
	}
	return tot
}

func Permute(skip func([]int) bool, dimensions ...int) [][]int {
	return permute(skip, dimensions, []int{})
}

func permute(skip func([]int) bool, dimensions []int, prefix []int) [][]int {
	set := make([][]int, 0)

	if len(dimensions) == 1 {
		for i := 0; i < dimensions[0]; i++ {
			val := append(append([]int{}, prefix...), i)
			set = append(set, val)
		}
		return set
	}

	max := dimensions[0]
	for i := 0; i < max; i++ {
		newprefix := append(prefix, i)
		if skip(newprefix) {
			continue
		}
		set = append(set, permute(skip, dimensions[1:], newprefix)...)
	}
	return set
}
