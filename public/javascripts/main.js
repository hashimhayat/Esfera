
"use strict";

function Performance() {

	var self = this;
	self.health = 0;
	self.bandwidthUsage = 0;
}

function Status() {

	var self = this;
	self.connected = false;
	self.streaming = false;
	self.findingParent = false;
	self.reconnecting = false;
}

function WRTCConnection(_config) {

	var self = this;

	// ID of the other peer.
	self.id = _config.id;
	self.otherPeer = _config.other;
	self.signalingChannel = _config.signalingChannel;
	self.dataChannel;
	self.stream;
	self.isbackup = false;

	self.config = { "iceServers": [{ "url": "stun:stun.1.google.com:19302" }] }; 
	self.connection = new RTCPeerConnection(self.config);

	self.getOther = function(){
		return self.otherPeer;
	}

	self.getStream = function(){
		return self.stream;
	}

	self.connection.onicecandidate = function (evt) {
		
		if (evt.candidate){
    		self.signalingChannel.emit('signal', { desc: 'forward', forwardType: "onicecandidate", from: self.id, to: self.otherPeer, message: evt.candidate });
		}	
	}

	self.connection.oniceconnectionstatechange = function(event) {
	  if (self.connection.iceConnectionState === "failed" || self.connection.iceConnectionState === "disconnected" || self.connection.iceConnectionState === "closed") {
	  	console.log(self.connection.iceConnectionState)
	  }
	};

    self.connection.onnegotiationneeded = () => {

        self.connection.createOffer()
        .then( function (offer) {
            return self.connection.setLocalDescription(offer);
        })
        .then( function() {
            console.log("Sending Offer")
            self.signalingChannel.emit('signal', { desc: 'forward', forwardType: "offer", from: self.id, to: self.otherPeer, message: self.connection.localDescription });
        })
        .catch(logError);
    }


	self.connection.ontrack = function (evt) {
        self.stream = evt.streams[0];
    };

    self.connection.onaddstream = function (evt) {
        self.stream = evt.stream;
    };
}

function Peer(config) {
	
	var self = this;
	self.broadcaster = config.broadcaster;
	self.MAX_CONNECTIONS = 5;
	self.status = new Status();
	self.performance = new Performance();

	// States
	self.streaming = false;
	
	self.connections = {};
	self.depth;

	// Socket Channel and Socket ID
	self.signalingChannel = io();
	self.id;

	self.stream;
	self.aspectRatio = { width: 500, height: 500 };
	self.streamAttached = false;
	self.viewView;

	/*
		Once the a new peer connects to the server. It has two options depnding on its status:
		If its a broadcaster:
			it tells the server that its a broadcaster
		Else if its not a broadcaster, or is a viewer client
			it sends a message to the moderator saying that I would like to connect.
			the moderator is responsible for telling the client who should he connect to.
	*/

	self.signalingChannel.on('connect', function (socket) {
	    
	    self.id = self.signalingChannel.id;

	    var sig = {};
		if (self.broadcaster){
			// Tell the server that I am a broadcaster
			sig = { desc: "broadcaster", from : self.id, to: "server", message: "I am the broadcaster." };
		} else {
			// Connection Request to the broadcaster
			//sig = { desc: "forward", forwardType: "request", from : self.id, to : "broadcaster", message: "I would like to connect." };
			sig = { desc: "moderator", from : self.id, to : "server", message: "I would like to connect." };
		}
		
		self.signalingChannel.emit('signal', sig);
	});

	/*
		The createConnection is responsible for creating a new WebRTC connection with
		a client who's socket ID is fed to the function. 
		The broadcaster inits a connection with the new client.
	*/

	self.createConnection = function(otherID) {

		var config = { id: self.id, other: otherID, signalingChannel: self.signalingChannel }
		var conn = new WRTCConnection(config);
		self.connections[otherID] = conn;

		// The addStream function triggers the new WebRTC connection setup between the two clients.
		if (self.broadcaster || self.stream)
			self.connections[otherID].connection.addStream(self.stream.clone());
	}

	self.createBackupConnection = function(otherID) {

		var config = { id: self.id, other: otherID, signalingChannel: self.signalingChannel }
		var conn = new WRTCConnection(config);
		self.connections[otherID] = conn;
		self.connections[otherID].isbackup = true;

		// The addStream function triggers the new WebRTC connection setup between the two clients.
		self.connections[otherID].connection.addStream(self.stream.clone());
	}

	self.parentdied = function(parentID){
		
		self.streamAttached = false
		delete self.connections[parentID];

		for (var conn in self.connections) {
	    	if (self.connections.hasOwnProperty(conn)) {
	        	self.stream = self.connections[conn].getStream();
		        self.viewStream("videoView");
				self.streaming = true;
	        	console.log("Stream set to:", conn);
	        	return conn;
	        }
	    }
	}

	/*
		This function is responsible in fetching the user's media and storing the stream locally
		 (If the user is a broadcaster), otherwise it just displays the stream to the users DOM.
	*/

	self.createStream = function(height, width, dom_ele) {

		self.aspectRatio = { width: width, height: height };
		self.viewView = dom_ele;

		if (self.broadcaster) {
			
			var getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia;
			var constraints = { video: self.aspectRatio, audio: false };
			
			if (getUserMedia) {
			    
			    getUserMedia = getUserMedia.bind(navigator);
				getUserMedia(constraints, 
				    
				    (stream) => {
				    	self.stream = stream.clone();
						self.viewStream(dom_ele);
				    }, 
				    (error) => {
				    	
				    }
				)			
			}
		}
	}

	self.viewStream = function(dom_ele){
		
		self.viewView = dom_ele;
		if (!self.streamAttached){
			var video = document.getElementById(dom_ele);
			video.setAttribute('height', self.aspectRatio.height + '');
			video.setAttribute('width', self.aspectRatio.width + '');
			var objectURL = window.URL || window.webkitURL;

		    try {
			  video.srcObject = self.stream;
			} catch (error) {
			  video.src = URL.createObjectURL(mediaSource);
			}

		    video.play(); 
		    self.streamAttached = true;
		}
	}


	self.signalingChannel.on('signal', function(in_signal)  {
		
	    var signal = in_signal;

	    if (signal.desc) {
	        
	        var desc = signal.desc;

	        switch (desc) {
	        	
	        	case "broadcaster":
	        		// Message for the broadcaster
	        		console.log(signal.message)
	        		break;

	        	case "request":

	        		// Request from a client to connect.
	        		console.log(signal.from, " would like to connect.");
	        		// Set up a new WebRTC connection.
                    self.createConnection(signal.from);
	        		break;

	        	case "backup":

	        		// A client wants to join you as a backup connection.
	        		console.log(signal.from, "sent a backup request");

	        		// Set up a new WebRTC Backup connection.
	        		self.createBackupConnection(signal.from);
	        		break;

	        	case "offer":
					
					// Set up a new WebRTC connection.
					self.createConnection(signal.from);

					self.connections[signal.from].connection.setRemoteDescription(signal.message).then(function () {
		            	console.log("Creating Answer")
		                return self.connections[signal.from].connection.createAnswer();
		            }).then((answer) => {
		                return self.connections[signal.from].connection.setLocalDescription(answer);
		            }).then(() => {
		            	console.log("Sending Answer")
		                self.signalingChannel.emit('signal', { desc: "forward", forwardType: "answer", to: signal.from, from: self.id, message: self.connections[signal.from].connection.localDescription });
		            }).catch(logError);	        		
		            
		            break;
	        	
	        	case "answer":
	        		
	        		console.log("Receiving Answer")

		            self.connections[signal.from].connection.setRemoteDescription(signal.message).catch(logError);
	        		break;

	        	case "onicecandidate":

	        		console.log("On Ice Connection")
	        		self.connections[signal.from].connection.addIceCandidate(signal.message).catch(logError);
	        		
	        	case "connected":
	        		
		        	console.log("Connection established with:", self.connections[signal.from].getOther());
		        	console.log("My Connections: ", self.connections);

		        	if (!self.streaming && self.connections[signal.from].getStream()){
		        		self.stream = self.connections[signal.from].getStream();
		        		self.viewStream("videoView");
						self.streaming = true;

						// Signaling my connected status.
						let info_signal = { desc: "information", type: "connected", from : self.id, with: self.connections[signal.from].getOther(), to: "server" }
						self.signalingChannel.emit('signal', info_signal);

						// HTML content for DEBUG
						document.getElementById("myid").innerHTML = self.id;
						document.getElementById("connectedto").innerHTML = self.connections[signal.from].getOther();
		        	}

		        	if (self.connections[signal.from].isBackup()){
		        		let info_signal = { desc: "information", type: "newbackup", from : self.id, backup: self.connections[signal.from].getOther(), to: "server" }
						self.signalingChannel.emit('signal', info_signal);
		        	}

	        		break;	

	        	case "parentdied":
	        		
	        		// Signal received when your parent dies.
	        		console.log(signal.from, " is dead.");

	        		// Connect to a new parent
	        		let newParent = self.parentdied(signal.from);

	        		// Inform the server about the new parent.
	        		let info_signal = { desc: "information", type: "newparent", from : self.id, parent: newParent, to: "server" }
					self.signalingChannel.emit('signal', info_signal);

	        		break;
	        	
	        	case "childdied":

	        		break;

	        	case "backupdied":

	        		break;

	        	case "error":
	        		console.log("Error: ", signal.desc.message);
	        		break;

	        	default:
	            	
	            	console.warn("Unsupported SDP type: ", signal.desc);
	        		break;
	        }

	    } else {
	    	console.log("adding IceCandidate")
	        self.connections[peerID].addIceCandidate(new RTCIceCandidate(signal.candidate)).catch(logError);
	    }
	});
}

function logError(error) {
    console.log(error.name + ": " + error.message);
}


document.addEventListener('DOMContentLoaded', main);

function main(){

	var peer;
	var isbroadcaster = false;

	if (window.location.hash == "#broadcaster"){
		isbroadcaster = true;
	} 

	peer = new Peer({broadcaster : isbroadcaster});
	peer.createStream(500, 500, "videoView");

}























