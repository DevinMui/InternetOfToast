var express = require("express")
var body_parser = require("body-parser")
var ejs = require("ejs")
var app = express()

var progress = false

var json = {} // this json structure tells the hardware what to make

app.use(body_parser.urlencoded({ extended: true }));
app.use(body_parser.json());
app.use(express.static('public'))
app.set('view engine', ejs)
app.set('json spaces', 2);

/*
 *
 * This function will allow me to write the toaster
 * application easier/make API requests with the server or whatever
 *
 * */
function makeToast(amount, color){
	json = { "amount": amount, "color": color } 
}

app.get('/', function(req, res){
	res.render("index.ejs") 
})

/*
 *
 * This post method will receive parameters such as
 * amount and color and will tell the toaster to make
 * toast according to these parameters
 *
 * */
app.post('/toast', function(req, res){
	if(!progress){
		var amount = req.body.amount
		//console.log(amount)
		if(!(amount <= 0)){
			var color = req.body.color
			if(color == "light" || color == "medium" || color == "dark"){
				res.status(200)
				progress = true
				makeToast(amount, color)
				res.json(json)
			} else {
				res.status(400)
				res.json("We can't make a " + color + " toast")
			}
		} else {
			res.status(400)
			res.json("I don't think we can make negative amounts of toast")
		}
	} else {
		res.status(400)
		res.json("The toaster is already making a toast")
	}
})

/*
 *
 * This get should be used by toaster server in order
 * to know when to make toast
 *
 * */
app.get('/toast_progress', function(req, res){
	console.log(progress)
	if(progress)
		res.json({ "progress": progress, "toast": json })
	else
		res.json({ "progress": progress })
})

app.post('/toast_progress', function(req, res){
	console.log(progress)
	if(progress)
		res.json({ "progress": progress, "toast": json })
	else
		res.json({ "progress": progress })
})

app.get('/toast_end', function(req, res){
	if(progress){
		progress = false
		json = {}
		res.json(progress)
	} else {
		res.status(400)
		res.json("We can't turn off a toaster if it's already off")
	}
})

app.post('/toast_end', function(req, res){
	if(progress){
		progress = false
		json = {}
		res.json(progress)
	} else {
		res.status(400)
		res.json("We can't turn off a toaster if it's already off")
	}
})

app.listen(3000, function(){
	console.log("Running on port 3000")
})