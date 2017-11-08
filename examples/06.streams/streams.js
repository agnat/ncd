const streams = require('./build/Release/streams')

streams.secondExport(streams, require('util'), require('stream'))

console.log(streams)

var w = streams.makeWriteStream()

w.write("foo", "utf8", () => { console.log('written') })
