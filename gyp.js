const path = require('path')
  , directory = path.relative('.', __dirname)
console.log(path.join(directory, 'include'));
