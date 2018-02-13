


module.exports = function (io) {

    var broadcaster = false;
    var clients = {};

    io.on('connection', function(socket) {

        console.log(socket.id, "connected.")
        clients[socket.id] = socket;

        // Receiver connects and ask for a handshake request
        socket.on('signal', function (signal) {

            switch (signal.desc) {
                
                case "broadcaster":

                    clients['broadcaster'] = socket;
                    broadcaster = true;

                    console.log("We have a broadcaster at: ",  clients['broadcaster'].id);

                    var sig = { desc: "broadcaster", from : "server", to : "broadcaster", message: "Broadcaster Ready."};
                    clients['broadcaster'].emit('signal', sig);
                    
                    break;
                
                case "forward":

                    switch (signal.to) {
                        
                        case "all":
                            
                            socket.broadcast.emit('signal', signal);
                            break;
                        
                        case "broadcaster":
                            
                            if (broadcaster){
                                clients['broadcaster'].emit('signal', { desc : "request", from : socket.id, to: "broadcaster"});
                            } else {
                                clients[socket.id].emit('signal', { desc: "error", from : "server", to: socket.id , message: "Broadcaster not available."});
                            }
                            break;

                        default:

                            signal.desc = signal.forwardType;
                            if (clients[signal.to])
                                clients[signal.to].emit('signal', signal);
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

        socket.on('disconnect', function(){
            console.log(socket.id,' disconnected');
        });

    });
}