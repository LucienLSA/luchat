new Vue({
    el: '#app',

    data: {
        ws: null, // Our websocket
        newMsg: '', // Holds new messages to be sent to the server
        chatContent: '', // A running list of chat messages displayed on the screen
        username: null, // Our username
        joined: false, // True if email and username have been filled in
        onlineUsers: [] // List of online users
    },

    created: function() {
        var self = this;
        this.ws = new WebSocket('ws://' + window.location.host + '/ws');
        this.ws.addEventListener('message', function(e) {
            console.log("message:", e);
            var msg = JSON.parse(e.data);
            if (msg != null) {
                // Handle different message types
                if (msg.message) {
                    // Public chat message
                    var msgObj = msg.message;
                    if (msgObj.filelink == "" || !msgObj.filelink) {
                        self.chatContent += '<div class="chip">' + msgObj.userphone + '</div>'
                            + emojione.toImage(msgObj.message) + '<br/>' + '<div class="time" style="color:gray">' + (msgObj.time || self.getCurrentTime()) + '</div>';
                    } else {
                        self.chatContent += '<div class="chip">'
                                + msgObj.userphone + '</div>' 
                            + emojione.toImage(msgObj.message) + '<br/>' + '&nbsp;&nbsp;&nbsp;&nbsp;<a href=' + msgObj.filelink + '>' 
                            + '[文件]' + '</a>' + '<div class="time" style="color:gray">' + (msgObj.time || self.getCurrentTime()) + '</div>';
                    }
                } else if (msg.online) {
                    // Online user notification
                    var onlineUser = msg.online;
                    // Add to online users list if not already present
                    var exists = self.onlineUsers.some(function(user) {
                        return user.userid === onlineUser.userid;
                    });
                    if (!exists) {
                        self.onlineUsers.push(onlineUser);
                    }
                }
            }

            var element = document.getElementById('chat-messages');
            element.scrollTop = element.scrollHeight; // Auto scroll to the bottom
        });
    },

    methods: {
        send: function () {
            var curTime = this.getCurrentTime();
            if (this.newMsg != '') {
                this.ws.send(
                    JSON.stringify({
                        message: {
                            userid: 'web-' + Date.now(), // Generate unique web user ID
                            userphone: '网页-' + this.username,
                            filelink: "",
                            message: $('<p>').html(this.newMsg).text(), // Strip out html
                            time: curTime
                        }
                    })
                );
                this.newMsg = ''; // Reset newMsg
            }
        },

        join: function () {
            if (!this.username) {
                Materialize.toast('请设定用户名', 2000);
                return
            }
            this.username = $('<p>').html(this.username).text();
            this.joined = true;
            
            // Send online notification to match desktop client format
            this.ws.send(JSON.stringify({
                online: {
                    userid: 'web-' + Date.now(),
                    userphone: '网页-' + this.username
                }
            }));
        },

        gravatarURL: function(email) {
            //return 'http://www.gravatar.com/avatar/' + CryptoJS.MD5('');
        },

        getCurrentTime: function() {
            var date = new Date();
            var year = date.getFullYear();
            var month = date.getMonth() + 1;
            var day = date.getDate();
            var hour = date.getHours();
            var minute = date.getMinutes();
            var second = date.getSeconds()
            return year + '-' + month + '-' + day + ' ' + hour + ':' + minute + ':' + second;
        }
    }
});