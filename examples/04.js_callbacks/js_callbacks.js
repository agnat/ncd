const mainq = require('./build/Release/js_callbacks')

const dumpArgs = (...args) => { console.log(args) }
mainq.jsDoneHandlers(dumpArgs)
mainq.asyncFunctions(dumpArgs, () => {console.log("done")})

