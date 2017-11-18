const
  tap = require('tap'),
  cp = require('child_process'),
  fs = require('fs'),
  path = require('path'),
  glob = require('glob'),

  gyp = path.join(__dirname, '..', 'node_modules', 'node-gyp', 'bin', 'node-gyp.js'),
  example_root = path.join(__dirname, '..', 'examples'),
  example_dirs = glob.sync(path.join(example_root, '[0-9][0-9].*'))

var examples = {}
for (var i in example_dirs) {
  examples[example_dirs[i]] = glob.sync(path.join(example_dirs[i], '*.js'))
}

function gyp_build(t, directory, done) {
  fs.stat(path.join(directory, 'build'), function (err, stats){
    var opts = {cwd: directory}, args = []
    args.push(!err && stats.isDirectory() ? 'build' : 'rebuild')
    if (process.env.NCD_VERBOSE) {
      opts.stdio = 'inherit'
      args.push('--verbose')
    }
    cp.spawn(gyp, args, opts).on('close', (code) => {
      t.equal(code, 0, "building addon succeeded");
      done(null, directory)
    })
  });
}

tap.test("build test addons", function (t) {
  t.plan(1)
  gyp_build(t, __dirname, t.end.bind(t))
})

tap.test("build and run example addons", function (t) {
  var testCount = 0,
      doneCount = 0,
      run, 
      done = (code) => { 
        t.equal(code, 0, "running example succeeded");
        if (++doneCount == testCount) t.end()
      }
  for (var dir in examples) {
    testCount += examples[dir].length
    run = (error, dir) => {
      var opts = {cwd: dir}, args = [examples[dir]]
      if (process.env.NCD_VERBOSE) {
        opts.stdio = 'inherit'
      }
      cp.spawn("node", args, opts).on('close', done)
    }
    gyp_build(t, dir, run)
  }
  t.plan(testCount + example_dirs.length)
})
