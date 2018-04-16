
const Graph = require('../P2P/Graph.js').Graph;

module.exports = function (io) {

    // The clientGraph stores all the details about a client.
    const clientGraph = new Graph();

    // EXPERIMENT
    var experiment = false;
    var receivers = [];
    var TOPOLOGY = 14;

    var sum_time = 0;
    var cnt_time = 0;

    io.on('connection', function(socket) {

        // CONNECTION ESTABLISHED
        console.log(socket.id, "connected.")
        clientGraph.addClient_at(socket.id, socket);
        
        socket.on('disconnect', function(){

            // CLIENT DISCONNECTS
            console.log(socket.id,' disconnected');

            var parent = clientGraph.getParent(socket.id);
            var children = clientGraph.getChildren(socket.id);

            // Remove the client from the graph.
            clientGraph.removeClient_at(socket.id);

            // Inform all the children of the disconnected client, that their parent has died.
            // This will triger them to refer to their backup connections.
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

                            // EXPERIMENT
                            if (experiment){
                                receivers.push(signal.from);

                                if (receivers.length == TOPOLOGY){

                                    console.log("EXPERIMENT STARTED")
                                    for (var i = 0; i < receivers.length; i++){
                                        let depth = clientGraph.depthOfNode(receivers[i]);
                                        clientGraph.getClient_sock_at(receivers[i]).emit('signal', { desc : "start_logs", depth: depth, from : socket.id, to: receivers[i]});
                                    }

                                }

                                if (receivers.length > TOPOLOGY){
                                    clientGraph.getClient_sock_at(receivers[receivers.length - 1]).emit('signal', { desc : "close", depth: depth, from : socket.id, to: receivers[i]});
                                }
                            }
                     
                            
                            // Sending a list of my Backup Connections to myself.

                            /*
                            let backups = clientGraph.getBackupConnections(signal.from, depth, undefined);
                            clientGraph.getClient_sock_at(signal.from).emit('signal', { desc : "backups", backups: backups, from : "server", to: "signal.from"});
                            console.log("Backup: ", backups);
                            */

                            // Logging for Graph
                            clientGraph.writeLogs();
                            break;

                        case "logs":

                            clientGraph.write_exp_data(signal.data);
                            receivers.pop();

                            if (receivers.length == 0){
                                console.log("EXPERIMENT FINISHED")
                            }
                            break;

                        case "start_time":

                            sum_time += signal.data;
                            cnt_time += 1;

                            console.log("Average Time: ", sum_time/cnt_time);
                            break;

                        case "newparent":

                            // Assign a client a new Parent that was one of its backup connections.
                            clientGraph.assign_newParent(signal.from, signal.parent);
                            clientGraph.remove_backup(signal.from, signal.parent);

                            // Assign a new backup.
                            let bkups = clientGraph.getBackupConnections(signal.from, clientGraph.depthOfNode(signal.from), 1);

                            for (var i = 0; i < bkups.length; i++){
                                clientGraph.getClient_sock_at(bkups[i]).emit('signal', { desc : "backup", from : signal.from, to: bkups[i]});
                            }

                            // Logging for Graph
                            clientGraph.writeLogs();
                            break;
                        
                        case "newbackup":

                            console.log("New Backup: Client: ", signal.from, " Backup: ", signal.backup);
                            clientGraph.setBackupFor(signal.from, signal.backup);

                            break;

                        default:
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


