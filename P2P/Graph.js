var jsonfile = require('jsonfile')

class Client {

	constructor(socket_id, socket) {
		this.id = socket_id;
		this.socket = socket;
		this.connections = 0;
		this.children = new Map();
		this.parents = new Map();
		this.isBroadcaster = false;
		this.depth = 0;
    }

    addParent(parent){
    	this.parents.set(parent, true);
    }

    hasParent(){
    	return (this.parents.size > 0 ? true : false);
    }

    getFirstParent(){
    	var itr = this.parents.keys();
    	let p = itr.next().value;
		return p;
    }

    addChild(child){
    	this.children.set(child, true);
    }
}

class Graph {

	constructor() {
		this.nodes = new Map();
		this.broadcaster = undefined;
		this.connections_available = new Map();		// {socket id: depth}
		this.max_connections = 3;
		this.max_backup = 1;
    }

    /*
		Creates a new client and adds it to the graph.
    */

    addClient_at(socket_id, socket) {
    	let client =  new Client(socket_id, socket);
    	this.nodes.set(socket_id, client);
    }

    /*
		Returns the socket of the client if present.
		Else returns undefined.
    */

    getClient_sock_at(socket_id){
    	if (this.nodes.has(socket_id))
    		return this.nodes.get(socket_id).socket;
    	console.warn("Error:getClient_sock_at");
    }

    getClient_at(socket_id){
    	if (this.nodes.has(socket_id))
    		return this.nodes.get(socket_id);
    	console.warn("Error:getClient_at");
    }

    hasClient_at(socket_id){
    	return this.nodes.has(socket_id);
    }

    removeClient_at(socket_id){
    	
    	// Remove from availble nodes
    	this.connections_available.delete(socket_id);

    	// Remove this node as a child of its parents.
    	// Decrement the availability of its parents, and add them to available.

    	this.nodes.get(socket_id).parents.forEach ((tf, parent_id, map) => {
    		this.getClient_at(parent_id).children.delete(socket_id);
    		this.getClient_at(parent_id).connections -= 1;
    	});



    	// Remove this node as a parent of its children
    	// Connect children to someone else.

    	this.nodes.get(socket_id).children.forEach ((tf, child_id, map) => {
    		this.getClient_at(child_id).parents.delete(socket_id);

    		// NEW CONNECTION
    	});

    	// Remove it from this.nodes
    	this.nodes.delete(socket_id);
    }

    /*
		Sets the Broadcaster.
    */

    setBroadcaster_at(socket_id){    	
    	if (this.nodes.has(socket_id)){
    		this.nodes.get(socket_id).isBroadcaster = true;
    		this.broadcaster = socket_id;
    		return true;
    	}
    	console.warn("Error:setBroadcaster_at");
    }

    getBroadcaster_sock(){

    	if (this.broadcaster){
    		return this.nodes.get(this.broadcaster).socket;
    	}
    	console.warn("Error:getBroadcaster_sock");
    }

    hasBroadcaster(){
    	return (this.broadcaster != undefined ? true : false)
    }

    connectionEstablished(parent, child){

    	if (this.nodes.has(parent) && this.nodes.has(child)){
    		this.nodes.get(parent).addChild(child);
    		this.nodes.get(child).addParent(parent);
    		return
    	}
    	console.warn("Error:connectionEstablished");
    }

    addAvailableConnections(socket_id, depth){
    	this.connections_available.set(socket_id, depth);
    	this.getClient_at(socket_id).depth = depth;
    	console.log("Total Available: ", this.connections_available.size)
    }

    getAvailableConnections(){
    	return connections_available;
    }

    inAvailableConnections(socket_id){
    	return this.connections_available.has(socket_id);
    }

    connectionsAvailable(){

    	console.log("Connections Available: ", this.connections_available.values())
    	return (this.connections_available.size > 0 ? true : false)
    }

    depthOfNode(socket_id){

    	if (this.nodes.has(socket_id) && this.getClient_at(socket_id).hasParent()){
    		return 1 + this.depthOfNode(this.getClient_at(socket_id).getFirstParent())
    	}

    	return 0
    }

    sort_availables(){
    	this.connections_available[Symbol.iterator] = function* () {
    		yield* [...this.entries()].sort((a, b) => a[1] - b[1]);
		}
    }

    getBackupConnection(id, depth){

    	var removeables = []
    	var iterator = this.connections_available[Symbol.iterator]();
    	var backup_nodes = [];
    
		for (let socket_id of iterator) {

			var client = this.getClient_at(socket_id[0]);

			if (client.depth <= depth) {

				if (client.connections < this.max_connections){

					if (id != socket_id[0]) {
						
						backup_nodes.push(socket_id[0]);
 						this.getClient_at(socket_id[0]).connections += 1;
 				
	 					if (this.getClient_at(socket_id[0]).connections >= this.max_connections){
	 						removeables.push(socket_id[0])
	 					}

	 					if (backup_nodes.length == this.max_backup){
	 						break;
	 					}
					}

	 			} else {
	 				removeables.push(socket_id[0])
				}

			} else {
				break;
			}
		}

    	for (var i = 0; i < removeables.length; i++){
   			this.connections_available.delete(removeables[i]);
 		}

 		return backup_nodes;
    }

    getAvailableConnection(){

    	var availableClient = undefined;
    	var removeables = []
 		
    	var iterator = this.connections_available[Symbol.iterator]();

    	// Find the node with the min connections in the current depth.
    	// if all nodes occupied at the current depth, increment depth.

		for (let socket_id of iterator) {

 			var client = this.getClient_at(socket_id[0]);

 			if (client.connections < this.max_connections){
 				
 				this.getClient_at(socket_id[0]).connections += 1;

 				if (this.getClient_at(socket_id[0]).connections >= this.max_connections){
 					removeables.push(socket_id[0])
 				}

 				availableClient = socket_id[0];
 				break;
 			} else {
 				removeables.push(socket_id[0])
 			}
 		}

 		for (var i = 0; i < removeables.length; i++){
   			this.connections_available.delete(removeables[i]);
 		}

   		if (availableClient != undefined)
   			return availableClient;
    }	

    getNodes(){

    	var nodes = [];
    	var itr = this.nodes[Symbol.iterator]();

		for (let socket_id of itr) {

			let g = this.getClient_at(socket_id[0]).depth;
			let n = {id: socket_id[0], group: g}
			nodes.push(n)
		}

		return nodes;
    }

    getEdges(){

    	var edges = [];
    	var itr = this.nodes[Symbol.iterator]();

		for (let socket_id of itr) {
		
			let itrchildren = socket_id[1].children;

			for (let child_id of itrchildren) {
				let e = {source: socket_id[0], target: child_id[0], value: 1 }
				edges.push(e);
			}
		}
		
		return edges;
    }

    writeLogs(){

    	var data = {nodes: this.getNodes(), links: this.getEdges()};

		var file = '/Users/hashimhayat/Desktop/Capstone/Spring/Esfera/public/data/data.json';
		jsonfile.writeFile(file, data, function (err) {
		  //console.error(err)
		});
    }

}

module.exports = {
	Client: Client,
	Graph: Graph
};
















