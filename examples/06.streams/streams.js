const streams = require('./build/Release/streams')

streams.secondExport(streams, require('util'), require('stream'))

console.log(streams)

var w = streams.makeWriteStream({objectMode: true})

w.write(7, null, () => { console.log('written') })
w.write(5, null, () => { console.log('written') })
w.end(null, null, () => { console.log('done') })
