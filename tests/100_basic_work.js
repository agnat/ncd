const
  basic_work = require('./build/Release/100_basic_work'),
  tap = require('tap')

tap.test("work queue: many items", function (t) {
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

tap.test((t) => {
  t.plan(1)
  basic_work.testStringResult((result) => {
    t.equal(result, "This is fine.")
    t.end()
  })
})

tap.test((t) => {
  t.plan(1)
  basic_work.testDoubleResult((result) => {
    t.equal(result, 0.5)
    t.end()
  })
})

tap.test((t) => {
  var count = 50000, done = 0
  t.plan(2 * count)
  function handler(success) {
    if (success) {
      return (error, result) => {
        t.equal(error, null)
        t.equal(result, "This is fine.")
        if (++done == count) { t.end() }
      } 
    } else {
      return (error, result) => {
        t.equal(error.message, "Kaputt.")
        t.equal(result, null)
        if (++done == count) { t.end() }
      }
    }
  }
  for (var i = 0; i < count; ++i) {
    var succeed = Math.random() < 0.5
    basic_work.testStringOrError(succeed, handler(succeed))
  }
})
