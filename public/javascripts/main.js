
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

 //    self.connection.addStream = function(stream) {
 //  		stream.getTracks().forEach(track => self.connection.addTrack(track, stream));
 //  		self.stream = stream;
	// };

	self.connection.ontrack = function (evt) {
		console.log(evt.streams);
        self.stream = evt.streams[0];
    };

    self.connection.onaddstream = function (evt) {
		console.log(evt);
        self.stream = evt.stream;
        console.log(self.stream.getTracks())
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

	self.signalingChannel.on('connect', function (socket) {
	    
	    self.id = self.signalingChannel.id;

	    var sig = {};
		if (self.broadcaster){
			// Tell the server that I am a broadcaster
			sig = { desc: "broadcaster", from : self.id, to: "server", message: "I am the broadcaster." };
		} else {
			// Connection Request to the broadcaster
			sig = { desc: "forward", forwardType: "request", from : self.id, to : "broadcaster", message: "I would like to connect." };
		}
		
		self.signalingChannel.emit('signal', sig);
	});

	self.createConnection = function(otherID) {
		var config = { id: self.id, other: otherID, signalingChannel: self.signalingChannel }
		var conn = new WRTCConnection(config);
		self.connections[otherID] = conn;

		if (self.broadcaster || self.stream)
			// self.stream.getTracks().forEach(function(track) {
			//     console.log(track)
			//});
			self.connections[otherID].connection.addStream(self.stream.clone());
	}

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
		    video.src = objectURL.createObjectURL(self.stream);
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
	        		console.log(signal.from, " would like to connect.")
	        		// Set up a new WebRTC connection.
                    self.createConnection(signal.from);
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

		        	if (!self.streaming && self.connections[signal.from].getStream()){
		        		self.stream = self.connections[signal.from].getStream();
		        		self.viewStream("videoView");
						self.signalingChannel.emit('signal', { desc: "broadcaster", from : self.id, to: "server", message: "I am the broadcaster." });
						self.streaming = true;
		        	}

	        		break;	

	        	case "close":
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























