import * as utilities from "./utilities.js";

// A = string
type A = Awaited<Promise<string>>;
// B = number
type B = Awaited<Promise<Promise<number>>>;
// C = boolean | number
type C = Awaited<boolean | Promise<number>>;

/*
 * Multi line comment
 */
function do_thing(x: any, y: number) {
	const $x = 12;
	
	const blankSpace = `
	first
	    second
	      third
	        
	`;
	
	// single line comment
	return Math.round($x * x / y);
}


const z = (stuff: Function) => {
	var more_stuff = "a string" 
		+ ' another string'
		+ `another ${with template} string`;

	return more_stuff;
}

async function main() {
	const response = await fetch("...");
    const greeting = await response.text();
    x = NaN;
    console.log(greeting);
}

class ClassWithPrivateAccessor {
	#message;
  
    get #decoratedMessage() {
        return `🎬${this.#message}🛑`;
	}
    set #decoratedMessage(msg) {
    	this.#message = msg;
    }
                  
    constructor() {
    	this.#decoratedMessage = 'hello world';
        console.log(this.#decoratedMessage);
    }
}

export { utilities };
