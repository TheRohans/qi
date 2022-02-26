/*
 * Multi line comment
 */

function do_thing(x: any, y: number) {
	const $x = 12;
	// single line comment
	return Math.round($x * x / y);
}

const z = (stuff) => {
	var more_stuff = "a string" 
		+ ' another string'
		+ `another ${with template} string`;

	return more_stuff;
}
