//Vi tok utgangspunkt i WebRTC video chat tutorial-en fra: "https://www.scaledrone.com/blog/webrtc-tutorial-simple-video-chat/"

//This will generate a room name if needed
if (!location.hash)
{
    location.hash = Math.floor(Math.random() * 0xFFFFFF).toString(16);
}
//part of the room identifier
const roomHash = location.hash.substring(1);

//The channel ID for the video
const scaleDrone = new ScaleDrone('yiS12Ts5RdNhebyM');
//Room name needs to be prefixed with 'observable-'
/**
 * 'roomName' adn 'roomHash' allows us to support multiple calls. A URL-hash will be generated for each room.
 * The URL can be sent to another peer to create a connection.
 * When we demonstrate the application, we only send the (often) 6-digit/character part after the # at the end of the URL
 */
const roomName = 'observable-' + roomHash;

//We use our STUN-server
const configuration = {
    iceServers:
    [
        {
            urls: 'stun:192.168.0.125:8000'
        }
    ]
};


//Global variables
let room;
let webRTCConnection;
let chatChannel;
let messages = [];
let sendButton;
let leaveChatButton;

//When the buttons are clicked, the correct method for that button is called
document.getElementById("sendButton").onclick = sendChatMessage;
document.getElementById("leaveChatButton").onclick = leaveChat;
//This is to make sure the text-area is clear when we start the application, so that the placeholdertext can show.
document.getElementById("text-write-area").value = "";

/**
 * This and the 'onError'-method creates a cleaner callbacks handling
 */
function onSuccess() {};

/**
 * Method that logs a possible error
 */
function onError(error)
{
    console.error(error);
};

scaleDrone.on('open', error => {
    if (error)
    {
        return console.error(error);
    }
    //By using scaledrone, we subscribe to a room (by the roomName), and then it will broadcast the messages sent to that room to all peers conected/subscribed to it
    room = scaleDrone.subscribe(roomName);
    room.on('open', error => {
        if (error)
        {
            onError(error);
        }
    });
    //If there are no errors, we will be connected to the room, and it will log/print an array of everyone connected to it
    //this includes us. Signaling server is now ready.
    room.on('members', members => {
        console.log('MEMBERS', members);
        //If we are the first peer to connect to the room, we wait until another one joins to recieve an offer
        //If we are the second peer in the room we will create the offer and signal this to the first peer
        const isOfferer = members.length === 2;
        startWebRTCConnection(isOfferer);
    });
});

//Method that sends the signaling data via Scaledrone to the other connection
function sendSignalingData(message)
{
    scaleDrone.publish({
      room: roomName,
      message
    });
}

/**
 * This function starts the WebRTC connection, generates ICE candidates for signaling to other peers,
 * (which we will pass to our signaling service), it then creates a chat channel for peers to send messages to each other.
 * The 'onnegotiationneeded' event, will be triggered if a change occurs which needs a session negootiation, e.g. video channel.
 * This will then start the createOffer-method, which will only be handled by the peer that is the offerer.
 * 'onaddstream' returns the video and audio streams of the remote user, which will be the source of the remote video element.
 * We then get the local video and audio through the 'getUserMedia'-method, and we set it as the source of the local video element,
 * and then we add it to the connection.
 *
 */
function startWebRTCConnection(isOfferer)
{
    webRTCConnection = new RTCPeerConnection(configuration);

    //'onicecandidate' will notify us whenever an ICE agent needs to deliver a message to the other peer through the signaling server
    webRTCConnection.onicecandidate = event => {
        if (event.candidate)
        {
            sendSignalingData({'candidate': event.candidate});
        }
    };

    //Create chatchannel to send messages between two peers
    createChatChannel();

    //If peer is offerer, then let the 'negotiationneeded' event create the offer and set the local description
    if (isOfferer)
    {
        webRTCConnection.onnegotiationneeded = () => {
          webRTCConnection.createOffer().then(localDescCreated).catch(onError);
        }
    }

    //When the remote stream arrives, it will be displayed in the #remoteVideo element
    webRTCConnection.ontrack = event => {
        const stream = event.streams[0];
        if (!remoteVideo.srcObject || remoteVideo.srcObject.id !== stream.id)
        {
            remoteVideo.srcObject = stream;
        }
    };

    //Get the peer's (our) local video and audio stream
    navigator.mediaDevices.getUserMedia({
      audio: true,
      video: true,
    })
    .then(stream => {
        //Display the peer's (our) local video in #localVideo element
        localVideo.srcObject = stream;
        //Add the local stream (our) to be sent to the conneting/other peer
        stream.getTracks().forEach(track => webRTCConnection.addTrack(track, stream));
    }, onError);

    //Listen to signaling data from Scaledrone
    room.on('data', (message, client) => {
        //If the message was sent by us, we return
        if (client.id === scaleDrone.clientId)
        {
            return;
        }

        if (message.sdp)
        {
            //calls method to check if it is the correct chat channel and to properly/finish setting this up
            acceptChatChannel();

            //The remote description will be set after recieving an offer from another peer
            webRTCConnection.setRemoteDescription(new RTCSessionDescription(message.sdp), () => {
                //we create an answer (if what we recieved was an offer) and sets the local description.
                if (webRTCConnection.remoteDescription.type === 'offer')
                {
                    webRTCConnection.createAnswer().then(localDescCreated).catch(onError);
                }
            }, onError);
        }
        else if (message.candidate)
        {
            //We add the new ICE candidate to the connections remote description
            webRTCConnection.addIceCandidate(new RTCIceCandidate(message.candidate), onSuccess, onError);
        }
    });
}

//Method that gets called when creating an offer and answering an offer. It will update the local description of the connection.
function localDescCreated(desc)
{
    webRTCConnection.setLocalDescription(desc, () => sendSignalingData({'sdp': webRTCConnection.localDescription}), onError);
}

//Method that creates the chat channel to send messages between peers on the connection.
function createChatChannel()
{
    chatChannel = webRTCConnection.createDataChannel('chat');
    //Any message sent on the channel will be displayed for all peers
    chatChannel.onmessage = (event) => {
        console.log('onmessage:', event.data);
        //Calls 'recieveChatMessage' to display the messages
        recieveChatMessage(event.data);
    };

    chatChannel.onopen = function(event)
    {
        //We check if the state of the channel is 'open', if it is, we can send messages and the negotiation process/process for the connection was sucessful
        var readyState = chatChannel.readyState;
        if (readyState == 'open')
        {
            console.log('chat channel is open');
        }
    }

    //When the channel is closed, we display that the connection is closed, and we disable the send button.
    chatChannel.onclose = () => {
        console.log('onclose');
        document.getElementById('display-messages').innerText += '\n--- Person B closed the chat ---';
        document.getElementById('sendButton').disabled = true;
    }
}

//Method that accepts the chat channel to send messages between peers on the connection.
//We do the same 'check if open', calls 'recieveChatMessage' to display messages to the peers, and disable the send button if connection is closed.
function acceptChatChannel()
{
    webRTCConnection.ondatachannel = (event) => {
        //checks if it is the correct channel
        if (event.channel.label == 'chat')
        {
            chatChannel = event.channel;
            chatChannel.onmessage = (event) => {
                console.log('onmessage:', event.data);
                recieveChatMessage(event.data);
            };
            chatChannel.onopen = function()
            {
                var readyState = chatChannel.readyState;
                if (readyState == 'open')
                {
                    //This will display to the peers on the connection that the channel is open
                    chatChannel.send('The chat channel is open! You can now send messages');
                }
            }
            chatChannel.onclose = () => {
                console.log('onclose');
                document.getElementById('display-messages').innerText += '\n--- You closed the chat ---';
                document.getElementById('sendButton').disabled = true;
            }
        }
    };
}

//This method is called when the 'Send' button is clicked. It retrieves the message from the text-area (where the peer writes their message)
//and both logs it and displays it to the display area.
//It then resets the text-area (where the peer writes their messages), so that it is empty
function sendChatMessage()
{
    let message = document.getElementById('text-write-area').value;
    console.log("Message: " + message);
    chatChannel.send(message);
    //For the person that sends the message, we add 'Me: ' before the message, to let the peers know who sent what
    document.getElementById('display-messages').innerText += '\nMe: ' + message;
    document.getElementById('text-write-area').value = "";
}

//Method that displays the recieved message to the display
function recieveChatMessage(message)
{
    //For the person that recieves the message, we add 'Person B: ' before the message, to let the peers know who sent what
    document.getElementById('display-messages').innerText += '\nPerson B: ' + message;
}

//This method is called when a peer clicks on the 'Leave Chat' button.
//It will close the data channel/chat channel, so the peers will no longer be able to communicate by chat.
function leaveChat()
{
    chatChannel.close();
}
