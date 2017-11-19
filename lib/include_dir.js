const path = require('path')
console.log(path.relative('.', path.resolve(__dirname, '..', 'include')))
