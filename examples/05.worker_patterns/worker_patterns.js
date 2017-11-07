const workers = require('./build/Release/worker_patterns')
  , EventEmitter = require('events')
  , ee = new EventEmitter()
    . on('progress', (p) => { console.log('progress', p) })
    . on('done',     ( ) => { console.log('done') })

workers.eventEmittingWorker(10000, 10, ee)

const progress = (i) => { console.log("progress", i) }
    , done = (...args) => { console.log("done", args) }

//=== Inline Worker ==========================================================
workers.inlineWorker(10000, 10, progress, done)

//=== Event Emitting Worker ==================================================

//=== Worker Component =======================================================
workers.workerComponent(10000, 10, ee)
