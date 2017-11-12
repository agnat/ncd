const mainq = require('./build/Release/main_queue')

mainq.mainQueueCallbacks()

var o = {}
mainq.pinnedObject(o)

var i = setInterval(() => {
  console.log(o)
  if (o.progress == 9) {
    clearInterval(i)
  }
}, 10);

