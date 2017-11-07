const worker_patterns = require('./build/Release/worker_patterns')
    , EventEmitter = require('events')

const progress = (i) => { console.log("progress", i) }
    , done = (...args) => { console.log("done", args) }

//=== Inline Worker ===================================
worker_patterns.inlineWorker(10000, 10, progress, done)

//=== Event Emitting Worker ===========================
const ee = new EventEmitter();
ee.on('progress', (i) => { console.log('progress', i) })
ee.on('done', (...args) => { console.log('done', args) })
worker_patterns.eventEmittingWorker(10000, 10, ee)

//=== Worker Component ================================
worker_patterns.workerComponent(10000, 10, ee)
