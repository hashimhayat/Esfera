var express = require('express');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index', { title: 'Esfera' });
});

router.get('/graph', function(req, res, next) {
  res.render('graph', { title: 'Esfera' });
});

module.exports = router;
