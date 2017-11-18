const
  tap = require('tap'),
  cp = require('child_process'),
  fs = require('fs')
  path = require('path')

tap.test(function (t) {
  t.plan(1)
  fs.stat(path.join(__dirname, 'build'), function (err, stats){
    var gyp = path.join(__dirname, '../node_modules/node-gyp/bin/node-gyp.js'),
      command = 'rebuild'
    if (!err && stats.isDirectory()) {
      command = 'build'
    }
    cp.spawn(gyp, [command], {stdio: 'inherit', cwd: __dirname})
      . on('close', (code) => { t.equal(code, 0); t.end() })
  });
})
