const
  basic_work = require('./build/Release/100_basic_work'),
  tap = require('tap')

tap.test(function (t) {
  var doneCount = 0,
    testItems = 100000,
    delay = 0
  t.plan(testItems + 1)
  basic_work.testWorkQueue(testItems, delay, () => { 
    t.pass("done callback called after work")
    if (++doneCount == testItems) {
      setTimeout(() => {
        t.ok(doneCount == testItems)
        t.end()
      }, 3 * delay + 100)
    }
  })
})

tap.test(function (t) {
  var doneCount = 0,
    testItems = 1000,
    delay = 1
  t.plan(testItems + 1)
  basic_work.testWorkQueue(testItems, delay, () => { 
    t.pass("done callback called after work")
    if (++doneCount == testItems) {
      setTimeout(() => {
        t.ok(doneCount == testItems)
        t.end()
      }, 3 * delay + 100)
    }
  })
})

