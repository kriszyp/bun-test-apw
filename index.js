let m = require('./build/Release/apw.node');
m.start((error, value) => {
	if (error)
		console.error(error)
	else
		console.log(value)
})