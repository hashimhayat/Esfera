var jsonfile = require('jsonfile')
var jsonexport = require('jsonexport');
var fs = require('fs');

class Client {

	constructor(socket_id, socket) {
		this.id = socket_id;
		this.socket = socket;
		this.connections = 0;
		this.children = new Map();
		this.parent = undefined;
		this.backups = new Map();
		this.backupFor = new Map();
		this.isBroadcaster = false;
		this.depth = 0;
    }

    addParent(parent){
    	this.parent = parent;
    }

    hasParent(){
    	return (this.parent == undefined ? false : true);
    }

    isParent(node){
    	return (this.parent == node ? true : false)
    }

    getParent(){
    	return this.parent;
    }

    addChild(child){
    	this.children.set(child, true);
    }

    getChildren(){
    	
    	var res = [];
    	var iterator = this.children[Symbol.iterator]();

    	for (let socket_id of iterator) {
    		res.push(socket_id[0]);
    	}

    	return res;
    }
}

class Graph {

	constructor() {
		this.nodes = new Map();
		this.connections_available = new Map();		// {socket id: depth}
		this.broadcaster = undefined;
		this.max_connections = 2;
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

    assign_newParent(socket_id, newParent){
    	this.getClient_at(socket_id).addParent(newParent);
    }

    remove_backup(socket_id, newParent){
    	this.getClient_at(socket_id).backups.delete(newParent);
    }

    getClient_at(socket_id){
    	if (this.nodes.has(socket_id))
    		return this.nodes.get(socket_id);
    	console.warn("Error:getClient_at");
    }

    hasClient_at(socket_id){
    	return this.nodes.has(socket_id);
    }

    getParent(socket_id){
    	if (this.nodes.has(socket_id) && this.nodes.get(socket_id).parent)
    		return this.nodes.get(socket_id).parent;
    	console.warn("Error:getParent");
    }

    getChildren(socket_id){
    	var res = []
    	
    	if (this.nodes.has(socket_id) && this.nodes.get(socket_id).children.size >= 0){
    		return this.nodes.get(socket_id).getChildren();
    	}

    	console.warn("Error:getParent");
    }

    setBackupFor(me, bfor){

    	if (this.hasClient_at(me)){
    		this.getClient_at(me).backupFor.set(bfor, true);
    	}
    	console.warn("Error:isbackupFor");
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
    		return 1 + this.depthOfNode(this.getClient_at(socket_id).getParent())
    	}

    	return 0
    }

    getBackupConnections(id, depth, limit){

    	if (limit == undefined) {limit = this.max_backup;}

    	var removeables = []
    	var iterator = this.connections_available[Symbol.iterator]();
    	var backup_nodes = [];
    
		for (let socket_id of iterator) {

			var client = this.getClient_at(socket_id[0]);

			// If this is not me, or my original parent.
			if (id != socket_id[0] && !this.getClient_at(id).isParent(socket_id[0])) {

				// Only if client is at a lower depth that me.
				if (client.depth <= depth) {

					if (client.connections < this.max_connections){
							
						backup_nodes.push(socket_id[0]);
						this.getClient_at(socket_id[0]).connections += 1;
					
	 					if (this.getClient_at(socket_id[0]).connections >= this.max_connections){ removeables.push(socket_id[0]); }

	 					if (backup_nodes.length == this.max_backup || backup_nodes.length == limit){
	 						break;
	 					}			

		 			} else {
		 				removeables.push(socket_id[0])
					}

				} else {
					break;
				}
			}
		}

    	for (var i = 0; i < removeables.length; i++){
   			this.connections_available.delete(removeables[i]);
 		}

 		// Add backups to the node
 		for (var i = 0; i < backup_nodes.length; i++){
 			this.getClient_at(id).backups.set(backup_nodes[i], true);
 		}

 		return backup_nodes;
    }

    /*
		Removes a Client when it disconnects.
		Handles following cases:
			1. Remove from connections_available.
			2. Remove as a child of its parent
			3. Remove as connection for its backups
			4. Remove as a backup for other clients
			5. Remove as a parent of its children
    */

    removeClient_at(socket_id){
    	
    	// Remove from availble nodes
    	this.connections_available.delete(socket_id);

    	// Remove this node as a child of its parent.
    	// Decrement the availability of its parents, and add them to available.

    	var myParent = this.nodes.get(socket_id).parent;
    	
    	if (myParent != undefined && this.hasClient_at(myParent)) {
    		this.getClient_at(myParent).connections -= 1;
	    	this.getClient_at(myParent).children.delete(socket_id);

			if (this.getClient_at(myParent).connections < this.max_connections){
				let d = this.depthOfNode(myParent);
				this.addAvailableConnections(myParent, d);
			}
    	}

    	// Remove this node as a connection to its backups
    	// Decrement the availability of its backups, and add them to available.
    	this.nodes.get(socket_id).backups.forEach ((tf, backup_id, map) => {
    		
    		if (this.hasClient_at(backup_id)) {
    			this.getClient_at(backup_id).connections -= 1;
	    		this.getClient_at(backup_id).children.delete(socket_id);

	    		if (this.getClient_at(backup_id).connections < this.max_connections){
	    			let d = this.depthOfNode(backup_id);
					this.addAvailableConnections(backup_id, d);
	    		}
    		}
    		
    	});

    	// Remove this node as a backup for others connections.
    	this.nodes.get(socket_id).backupFor.forEach ((tf, backup_id, map) => {
    		
    		if (this.hasClient_at(backup_id)) {
    			this.getClient_at(backup_id).backups.delete(socket_id);
    		}
    		
    	});

    	// Remove this node as a parent of its children
    	// Connect children to someone else.
    	this.nodes.get(socket_id).children.forEach ((tf, child_id, map) => {

    		if (this.hasClient_at(child_id)) {
    			this.getClient_at(child_id).parent = undefined;
    		}
    	});

    	// Remove it from this.nodes
    	this.nodes.delete(socket_id);
    }

    getAvailableConnection(){

    	var output = undefined;
    	var res = [];
    	var removeables = [];
    	var iterator = this.connections_available[Symbol.iterator]();
    	var min_depth = Number.MAX_VALUE;
    	var min_conns = Number.MAX_VALUE;

    	// Find all nodes at the min depth
    	for (let socket_id of iterator) {

    		var client = this.getClient_at(socket_id[0]);

    		if (client.connections < this.max_connections){

    			if (client.depth <= min_depth){
    				min_depth = client.depth;
    				res.push(socket_id[0]);
    			} 

    		} else {
    			removeables.push(socket_id[0]);
    		}
    	}

    	// Find the node with min number of connections
    	for (var i = 0; i < res.length; i++){

    		var client = this.getClient_at(res[i]);

    		if (client.connections < min_conns){
    			min_conns = client.connections;
    			output = res[i]
    		}
    	}

    	// Update the connections of the choosen node.
    	this.getClient_at(output).connections += 1;

    	if (this.getClient_at(output).connections >= this.max_connections){
    		removeables.push(output);
    	}

    	for (var i = 0; i < removeables.length; i++){
   			this.connections_available.delete(removeables[i]);
 		}

 		return output;
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
				let e = {source: socket_id[0], target: child_id[0], value: 'black'};
				edges.push(e);
			}

			let itrbackup = socket_id[1].backups;

			for (let backup_id of itrbackup) {
				let e = {source: socket_id[0], target: backup_id[0], value: 'green'};
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

    write_exp_data(data){

        var dir = "/Users/hashimhayat/Desktop/Capstone_Exp/";
        var file = dir + data[0].depth + '-' + data[0].type + '-' +  data[0].id + '-' + data[0].sender + '.csv';
        
        jsonexport(data, function(err, csv){
            if(err) return console.log(err);
            
            fs.writeFile(file, csv, function(err) {
                if(err) {
                    return console.log(err);
                }
                console.log("The file was saved!");
            }); 
        });
    }
}

module.exports = {
	Client: Client,
	Graph: Graph
};
















