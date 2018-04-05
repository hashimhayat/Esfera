
const Graph = require('../P2P/Graph.js').Graph;

module.exports = function (io) {

    // The clientGraph stores all the details about a client.
    const clientGraph = new Graph();

    io.on('connection', function(socket) {

        console.log(socket.id, "connected.")

        clientGraph.addClient_at(socket.id, socket);
        clientGraph.connectionsAvailable();

        // A Client disconnects.
        socket.on('disconnect', function(){
            console.log(socket.id,' disconnected');

            var parent = clientGraph.getParent(socket.id);
            var children = clientGraph.getChildren(socket.id);
            clientGraph.removeClient_at(socket.id);

            // Telling all the children of the disconnected client, that their parent has died.
            for (var c = 0; c < children.length; c++){
                clientGraph.getClient_sock_at(children[c]).emit('signal', { desc : "parentdied", from : socket.id, to: children[c]});
            }

            clientGraph.writeLogs();
        });

        // Receiver connects and ask for a handshake request
        socket.on('signal', function (signal) {

            switch (signal.desc) {
                
                case "broadcaster":

                    clientGraph.setBroadcaster_at(socket.id)
                    clientGraph.addAvailableConnections(socket.id, 0);
                    
                    console.log("We have a broadcaster at: ",  clientGraph.getClient_at(socket.id).id);

                    var sig = { desc: "broadcaster", from : "server", to : "broadcaster", message: "Broadcaster Ready."};
                    clientGraph.getBroadcaster_sock().emit('signal', sig);
                    clientGraph.writeLogs();
                    
                    break;

                case "moderator":
                        
                    // The moderator is responsible for telling a new client that whom it should connect to.

                    if (clientGraph.connectionsAvailable()){
                        let sendingRequestTo = clientGraph.getAvailableConnection();
                        clientGraph.getClient_sock_at(sendingRequestTo).emit('signal', { desc : "request", from : socket.id, to: sendingRequestTo});

                    } else {
                        console.log("Connection not available.")
                        clientGraph.getClient_sock_at(socket.id).emit('signal', { desc: "error", from : "server", to: socket.id , message: "Connections not available."});
                    }
                    
                    break;

                case "information":

                    switch (signal.type) {
                        
                        case "connected":

                            console.log("Connection Established between: Parent: ", signal.with, " Child: ", signal.from);
                            
                            // A connection has been established between sender of this signal and another client.
                            clientGraph.connectionEstablished(signal.with, signal.from);

                            let depth = clientGraph.depthOfNode(signal.from);
                            
                            // Add to availaibility list
                            console.log("Depth: ", depth);
                            clientGraph.addAvailableConnections(signal.from, depth);
                            clientGraph.connectionsAvailable()
                            
                            // Backup Connections
                            var backups = clientGraph.getBackupConnection(signal.from, depth);

                            for (var i = 0; i < backups.length; i++){
                                clientGraph.getClient_sock_at(backups[i]).emit('signal', { desc : "backup", from : signal.from, to: backups[i]});
                            }

                            console.log("Backup: ", backups);

                            // Logging for Graph
                            clientGraph.writeLogs();
                            break;

                        default:
                            // statements_def
                            break;
                    }

                    break;
            
                case "forward":

                    switch (signal.to) {
                        
                        case "all":
                            
                            socket.broadcast.emit('signal', signal);
                            break;

                        default:

                            signal.desc = signal.forwardType;
                            if (clientGraph.hasClient_at(signal.to))
                                clientGraph.getClient_sock_at(signal.to).emit('signal', signal);
                            else
                                console.log("Client doesnt exists: ", signal.to);
                            break;
                    }

                    break; 
                default:
                    console.log("Unsupported signal type: ", signal.type)
                    break;
            }

        });

    });
}