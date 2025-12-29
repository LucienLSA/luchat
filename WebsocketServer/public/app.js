new Vue({
    el: '#app',

    data: {
        ws: null, // Our websocket
        newMsg: '', // Holds new messages to be sent to the server
        chatContent: '', // A running list of chat messages displayed on the screen
        username: null, // Our username
        joined: false, // True if email and username have been filled in
        onlineUsers: [], // List of online users
        heartbeatTimer: null, // 心跳定时器
        heartbeatInterval: 30000, // 心跳间隔（30秒）
        // 重连相关配置
        reconnectTimer: null, // 重连定时器
        reconnectAttempts: 0, // 重连尝试次数
        maxReconnectAttempts: 5, // 最大重连次数
        reconnectDelay: 1000, // 初始重连延迟（毫秒）
        maxReconnectDelay: 30000, // 最大重连延迟（毫秒）
        isReconnecting: false, // 是否正在重连
        connectionStatus: 'disconnected', // 连接状态：connecting, connected, reconnecting, disconnected
        // 文件上传相关
        uploadingFiles: [], // 正在上传的文件列表
        fileList: [], // 文件列表
        chunkSize: 5 * 1024 * 1024, // 分块大小 5MB
        maxFileSize: 100 * 1024 * 1024 // 最大文件大小 100MB
    },

    created: function() {
        this.connect();
    },

    mounted: function() {
        // 初始化Materialize组件
        var self = this;
        this.$nextTick(function() {
            // 初始化模态框
            try {
                $('.modal').modal({
                    dismissible: true,
                    opacity: 0.5,
                    inDuration: 300,
                    outDuration: 200,
                    ready: function() {
                        console.log('模态框初始化完成');
                    }
                });
                console.log('Materialize模态框初始化成功');
            } catch (error) {
                console.error('Materialize模态框初始化失败:', error);
            }
        });
    },

    methods: {
        // 建立WebSocket连接
        connect: function() {
        var self = this;
            this.connectionStatus = 'connecting';
        this.ws = new WebSocket('ws://' + window.location.host + '/ws');
            
        this.ws.onopen = function() {
            console.log('连接已建立，开始发送心跳');
                self.connectionStatus = 'connected';
                self.isReconnecting = false;
                self.reconnectAttempts = 0; // 重置重连次数
                
                // 每30秒发送一次ping
            self.heartbeatTimer = setInterval(function() {
                    if (self.ws && self.ws.readyState === WebSocket.OPEN) {
                    self.ws.send('ping');
                }
            }, self.heartbeatInterval);
        };
        this.ws.addEventListener('message', function(e) {
            console.log("message:", e);
            if (e.data === 'pong') {
                console.log('收到心跳回复');
                return; // 不处理心跳消息
            }
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
            
            this.ws.onclose = function(event) {
                console.log('连接已关闭', event.code, event.reason);
                self.connectionStatus = 'disconnected';
                self.clearTimers();
                
                // 如果不是主动关闭，则尝试重连
                if (event.code !== 1000 && !self.isReconnecting) {
                    self.attemptReconnect();
                }
        };
            
        this.ws.onerror = function(err) {
            console.error('WebSocket错误:', err);
                self.connectionStatus = 'disconnected';
                self.clearTimers();
                
                if (!self.isReconnecting) {
                    self.attemptReconnect();
                }
            };
        },

        // 尝试重连
        attemptReconnect: function() {
            var self = this;
            
            if (self.reconnectAttempts >= self.maxReconnectAttempts) {
                console.log('已达到最大重连次数，停止重连');
                self.connectionStatus = 'disconnected';
                Materialize.toast('连接失败，请刷新页面重试', 5000);
                return;
            }
            
            self.isReconnecting = true;
            self.connectionStatus = 'reconnecting';
            self.reconnectAttempts++;
            
            // 计算重连延迟（指数退避）
            var delay = Math.min(
                self.reconnectDelay * Math.pow(2, self.reconnectAttempts - 1),
                self.maxReconnectDelay
            );
            
            console.log('第' + self.reconnectAttempts + '次重连尝试，延迟' + delay + 'ms');
            Materialize.toast('连接断开，正在重连... (' + self.reconnectAttempts + '/' + self.maxReconnectAttempts + ')', 2000);
            
            self.reconnectTimer = setTimeout(function() {
                self.connect();
            }, delay);
        },

        // 清理定时器
        clearTimers: function() {
            if (this.heartbeatTimer) {
                clearInterval(this.heartbeatTimer);
                this.heartbeatTimer = null;
            }
            if (this.reconnectTimer) {
                clearTimeout(this.reconnectTimer);
                this.reconnectTimer = null;
            }
        },

        // 手动重连
        manualReconnect: function() {
            this.clearTimers();
            this.reconnectAttempts = 0;
            this.isReconnecting = false;
            this.connect();
        },

        // 获取连接状态文本
        getStatusText: function() {
            switch(this.connectionStatus) {
                case 'connecting': return '连接中...';
                case 'connected': return '已连接';
                case 'reconnecting': return '重连中...';
                case 'disconnected': return '已断开';
                default: return '未知';
            }
        },

        // 文件选择
        selectFile: function() {
            this.$refs.fileInput.click();
        },

        // 文件选择处理
        onFileSelect: function(event) {
            var files = event.target.files;
            for (var i = 0; i < files.length; i++) {
                this.uploadFile(files[i]);
            }
            // 清空input
            event.target.value = '';
        },

        // 上传文件
        uploadFile: function(file) {
            // 检查文件大小
            if (file.size > this.maxFileSize) {
                Materialize.toast('文件大小不能超过100MB', 3000);
                return;
            }

            // 检查文件类型
            var allowedTypes = ['image/jpeg', 'image/png', 'application/pdf', 'application/zip'];
            if (allowedTypes.indexOf(file.type) === -1) {
                Materialize.toast('不支持的文件类型', 3000);
                return;
            }

            var fileId = Date.now() + '_' + Math.random().toString(36).substr(2, 9);
            var uploadFile = {
                id: fileId,
                name: file.name,
                size: file.size,
                file: file,
                progress: 0,
                status: '准备中...',
                chunks: Math.ceil(file.size / this.chunkSize),
                uploadedChunks: 0,
                isResuming: false,
                isInstant: false
            };

            this.uploadingFiles.push(uploadFile);
            this.startChunkUpload(uploadFile);
        },

        // 开始分块上传
        startChunkUpload: function(uploadFile) {
            var self = this;
            var file = uploadFile.file;
            var totalChunks = uploadFile.chunks;
            var fileHash = this.generateFileHash(file.name + file.size + file.lastModified);

            uploadFile.status = '检查秒传...';

            // 先检查是否可以秒传
            this.checkInstantUpload(fileHash, file.name, file.size).then(function(instantInfo) {
                if (instantInfo.can_instant) {
                    console.log('检测到可秒传文件:', instantInfo);
                    uploadFile.status = '秒传完成';
                    uploadFile.progress = 100;
                    uploadFile.isInstant = true;
                    Materialize.toast('文件已存在，秒传成功！', 3000);
                    
                    // 发送WebSocket消息通知文件上传完成
                    self.sendMessage({
                        type: 'file',
                        filename: file.name,
                        filelink: instantInfo.file_url,
                        user: self.username
                    });
                    
                    // 从上传列表中移除
                    setTimeout(function() {
                        var index = self.uploadingFiles.indexOf(uploadFile);
                        if (index > -1) {
                            self.uploadingFiles.splice(index, 1);
                        }
                    }, 2000);
                    return;
                }
                
                // 不能秒传，检查断点续传
                uploadFile.status = '检查断点续传...';
                self.checkResume(fileHash, file.name, totalChunks, file.size).then(function(resumeInfo) {
                    if (resumeInfo.canResume) {
                        console.log('检测到可续传文件，已上传分块:', resumeInfo.uploadedChunks);
                        uploadFile.status = '断点续传中...';
                        uploadFile.isResuming = true;
                        Materialize.toast('检测到未完成的上传，开始断点续传', 2000);
                    } else {
                        uploadFile.status = '上传中...';
                        uploadFile.isResuming = false;
                    }
                    
                    // 开始上传（跳过已上传的分块）
                    self.uploadChunksWithResume(uploadFile, fileHash, totalChunks, resumeInfo.uploadedChunks);
                }).catch(function(error) {
                    console.error('检查断点续传失败:', error);
                    // 如果检查失败，按正常流程上传
                    uploadFile.status = '上传中...';
                    uploadFile.isResuming = false;
                    self.uploadChunksWithResume(uploadFile, fileHash, totalChunks, []);
                });
            }).catch(function(error) {
                console.error('检查秒传失败:', error);
                // 如果秒传检查失败，按正常流程上传
                uploadFile.status = '上传中...';
                uploadFile.isResuming = false;
                self.uploadChunksWithResume(uploadFile, fileHash, totalChunks, []);
            });
        },

        // 检查秒传
        checkInstantUpload: function(fileHash, filename, fileSize) {
            var self = this;
            return new Promise(function(resolve, reject) {
                $.ajax({
                    url: '/api/instant/check',
                    type: 'POST',
                    contentType: 'application/json',
                    data: JSON.stringify({
                        file_hash: fileHash,
                        filename: filename,
                        file_size: fileSize
                    }),
                    success: function(response) {
                        if (response.code === 200 && response.data) {
                            resolve(response.data);
                        } else {
                            reject(new Error('检查秒传失败'));
                        }
                    },
                    error: function(xhr, status, error) {
                        reject(error);
                    }
                });
            });
        },

        // 检查断点续传
        checkResume: function(fileHash, filename, totalChunks, fileSize) {
            var self = this;
            return new Promise(function(resolve, reject) {
                $.ajax({
                    url: '/api/resume/check',
                    type: 'POST',
                    contentType: 'application/json',
                    data: JSON.stringify({
                        file_hash: fileHash,
                        filename: filename,
                        total_chunks: totalChunks,
                        file_size: fileSize
                    }),
                    success: function(response) {
                        if (response.code === 200 && response.data) {
                            resolve(response.data);
                        } else {
                            reject(new Error('检查断点续传失败'));
                        }
                    },
                    error: function(xhr, status, error) {
                        reject(error);
                    }
                });
            });
        },

        // 带断点续传的分块上传
        uploadChunksWithResume: function(uploadFile, fileHash, totalChunks, uploadedChunks) {
            var self = this;
            var file = uploadFile.file;
            var uploadPromises = [];

            // 创建已上传分块的Set，便于快速查找
            var uploadedSet = new Set(uploadedChunks);

            // 只上传未完成的分块
            for (var i = 0; i < totalChunks; i++) {
                if (!uploadedSet.has(i)) {
                    var start = i * this.chunkSize;
                    var end = Math.min(start + this.chunkSize, file.size);
                    var chunk = file.slice(start, end);

                    uploadPromises.push(this.uploadChunk(uploadFile, i, chunk, fileHash, totalChunks));
                } else {
                    // 已上传的分块，直接更新进度
                    uploadFile.uploadedChunks++;
                    uploadFile.progress = Math.round((uploadFile.uploadedChunks / totalChunks) * 100);
                }
            }

            // 如果没有需要上传的分块，直接合并
            if (uploadPromises.length === 0) {
                console.log('所有分块已上传完成，开始合并文件');
                this.mergeChunks(uploadFile, fileHash, totalChunks);
                return;
            }

            // 等待所有分块上传完成
            Promise.all(uploadPromises).then(function() {
                // 合并文件
                self.mergeChunks(uploadFile, fileHash, totalChunks);
            }).catch(function(error) {
                console.error('分块上传失败:', error);
                uploadFile.status = '上传失败';
                Materialize.toast('文件上传失败', 3000);
            });
        },

        // 上传单个分块
        uploadChunk: function(uploadFile, chunkIndex, chunk, fileHash, totalChunks) {
            var self = this;
            return new Promise(function(resolve, reject) {
                var formData = new FormData();
                formData.append('file_hash', fileHash);
                formData.append('chunk_index', chunkIndex);
                formData.append('total_chunks', totalChunks);
                formData.append('filename', uploadFile.name);
                formData.append('chunk_data', chunk);

                $.ajax({
                    url: '/api/chunk',
                    type: 'POST',
                    data: formData,
                    processData: false,
                    contentType: false,
                    success: function(response) {
                        uploadFile.uploadedChunks++;
                        uploadFile.progress = Math.round((uploadFile.uploadedChunks / totalChunks) * 100);
                        resolve(response);
                    },
                    error: function(xhr, status, error) {
                        reject(error);
                    }
                });
            });
        },

        // 合并文件块
        mergeChunks: function(uploadFile, fileHash, totalChunks) {
            var self = this;
            $.ajax({
                url: '/api/merge',
                type: 'POST',
                contentType: 'application/json',
                data: JSON.stringify({
                    file_hash: fileHash,
                    filename: uploadFile.name,
                    total_chunks: totalChunks
                }),
                success: function(response) {
                    uploadFile.status = '上传完成';
                    uploadFile.progress = 100;
                    
                    // 发送文件消息到聊天
                    var filePath = '';
                    if (response.code === 200 && response.data && response.data.file_path) {
                        filePath = response.data.file_path;
                    } else {
                        console.error('合并文件响应格式错误:', response);
                        filePath = '/public/uploads/' + uploadFile.name;
                    }
                    
                    self.ws.send(JSON.stringify({
                        message: {
                            userid: 'web-' + Date.now(),
                            userphone: '网页-' + self.username,
                            filelink: filePath,
                            message: '[文件] ' + uploadFile.name,
                            time: self.getCurrentTime()
                        }
                    }));

                    // 3秒后移除上传项
                    setTimeout(function() {
                        var index = self.uploadingFiles.findIndex(function(f) {
                            return f.id === uploadFile.id;
                        });
                        if (index > -1) {
                            self.uploadingFiles.splice(index, 1);
                        }
                    }, 3000);

                    Materialize.toast('文件上传成功', 2000);
                },
                error: function(xhr, status, error) {
                    uploadFile.status = '合并失败';
                    Materialize.toast('文件合并失败', 3000);
                }
            });
        },

        // 生成文件哈希（优化版本）
        generateFileHash: function(str) {
            // 使用更复杂的哈希算法
            var hash = 0;
            if (str.length === 0) return hash.toString();
            
            for (var i = 0; i < str.length; i++) {
                var char = str.charCodeAt(i);
                hash = ((hash << 5) - hash) + char;
                hash = hash & hash; // Convert to 32bit integer
            }
            
            // 添加时间戳和随机数，确保唯一性
            var timestamp = Date.now().toString(36);
            var random = Math.random().toString(36).substr(2, 5);
            
            return Math.abs(hash).toString(36) + '_' + timestamp + '_' + random;
        },

        // 显示文件列表
        showFileList: function() {
            console.log('显示文件列表按钮被点击');
            this.loadFileList();
            
            // 延迟一点时间确保数据加载完成
            setTimeout(function() {
                console.log('尝试打开模态框...');
                var modal = document.getElementById('fileListModal');
                console.log('模态框元素:', modal);
                
                if (modal) {
                    // 创建模态框背景
                    var overlay = document.createElement('div');
                    overlay.className = 'modal-overlay';
                    overlay.style.cssText = 'position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.5); z-index: 1002;';
                    
                    // 设置模态框样式
                    modal.style.cssText = 'position: fixed; top: 10%; left: 50%; transform: translateX(-50%); background: white; border-radius: 4px; z-index: 1003; max-width: 80%; max-height: 80%; overflow-y: auto;';
                    modal.style.display = 'block';
                    
                    // 添加背景到页面
                    document.body.appendChild(overlay);
                    
                    // 点击背景关闭模态框
                    overlay.onclick = function() {
                        modal.style.display = 'none';
                        document.body.removeChild(overlay);
                    };
                    
                    // 添加关闭按钮事件
                    var closeBtn = modal.querySelector('.modal-close');
                    if (closeBtn) {
                        closeBtn.onclick = function() {
                            modal.style.display = 'none';
                            document.body.removeChild(overlay);
                        };
                    }
                    
                    console.log('模态框显示成功');
                } else {
                    console.error('找不到模态框元素');
                }
            }, 500);
        },

        // 加载文件列表
        loadFileList: function() {
            var self = this;
            console.log('开始加载文件列表...');
            $.ajax({
                url: '/api/files',
                type: 'GET',
                success: function(response) {
                    console.log('文件列表响应:', response);
                    // 后端返回的数据结构是 {code: 200, message: "success", data: {files: [...]}}
                    if (response.code === 200 && response.data && response.data.files) {
                        self.fileList = response.data.files;
                        console.log('文件列表加载成功，文件数量:', self.fileList.length);
                    } else {
                        self.fileList = [];
                        console.error('文件列表数据格式错误:', response);
                        Materialize.toast('文件列表数据格式错误', 3000);
                    }
                },
                error: function(xhr, status, error) {
                    console.error('加载文件列表失败:', error);
                    Materialize.toast('加载文件列表失败', 3000);
                }
            });
        },

        // 下载文件
        downloadFile: function(filename) {
            console.log('下载文件:', filename);
            if (!filename) {
                Materialize.toast('文件名无效', 3000);
                return;
            }
            window.open('/api/download?filename=' + encodeURIComponent(filename), '_blank');
        },

        // 关闭模态框
        closeModal: function() {
            var modal = document.getElementById('fileListModal');
            var overlay = document.querySelector('.modal-overlay');
            if (modal) {
                modal.style.display = 'none';
            }
            if (overlay) {
                document.body.removeChild(overlay);
            }
        },

        // 测试删除功能
        testDelete: function() {
            console.log('测试删除功能');
            if (this.fileList.length > 0) {
                var firstFile = this.fileList[0];
                console.log('测试删除文件:', firstFile.file_name);
                this.deleteFile(firstFile.file_name);
            } else {
                Materialize.toast('没有文件可以删除', 3000);
            }
        },

        // 重试上传
        retryUpload: function(uploadFile) {
            console.log('重试上传文件:', uploadFile.name);
            uploadFile.status = '重试中...';
            uploadFile.progress = 0;
            uploadFile.uploadedChunks = 0;
            uploadFile.isResuming = false;
            
            // 重新开始上传流程
            this.startChunkUpload(uploadFile);
        },

        // 删除文件
        deleteFile: function(filename) {
            var self = this;
            console.log('尝试删除文件:', filename);
            if (confirm('确定要删除文件 "' + filename + '" 吗？')) {
                $.ajax({
                    url: '/api/deletefile?filename=' + encodeURIComponent(filename),
                    type: 'DELETE',
                    success: function(response) {
                        console.log('删除文件响应:', response);
                        if (response.code === 200) {
                            Materialize.toast('文件删除成功', 2000);
                            self.loadFileList(); // 重新加载文件列表
                        } else {
                            Materialize.toast('文件删除失败: ' + (response.message || '未知错误'), 3000);
                        }
                    },
                    error: function(xhr, status, error) {
                        console.error('删除文件失败:', error);
                        console.error('响应状态:', xhr.status);
                        console.error('响应文本:', xhr.responseText);
                        Materialize.toast('文件删除失败: ' + error, 3000);
                    }
                });
            }
        },

        // 格式化文件大小
        formatFileSize: function(bytes) {
            if (!bytes || bytes === 0) return '0 Bytes';
            if (typeof bytes !== 'number' || isNaN(bytes)) return '未知大小';
            
            var k = 1024;
            var sizes = ['Bytes', 'KB', 'MB', 'GB'];
            var i = Math.floor(Math.log(bytes) / Math.log(k));
            return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
        },

        // 格式化日期
        formatDate: function(dateString) {
            if (!dateString) {
                return '未知时间';
            }
            
            var date = new Date(dateString);
            if (isNaN(date.getTime())) {
                return '无效时间';
            }
            
            return date.toLocaleDateString() + ' ' + date.toLocaleTimeString();
        },

        // 获取文件图标
        getFileIcon: function(filename) {
            // 检查文件名是否存在且为字符串
            if (!filename || typeof filename !== 'string') {
                return 'insert_drive_file';
            }
            
            var parts = filename.split('.');
            if (parts.length < 2) {
                return 'insert_drive_file';
            }
            
            var ext = parts.pop().toLowerCase();
            switch (ext) {
                case 'jpg':
                case 'jpeg':
                case 'png':
                case 'gif':
                    return 'image';
                case 'pdf':
                    return 'picture_as_pdf';
                case 'zip':
                case 'rar':
                    return 'archive';
                case 'txt':
                    return 'description';
                case 'doc':
                case 'docx':
                    return 'description';
                default:
                    return 'insert_drive_file';
            }
        },

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
        },
        beforeDestroy: function() {
            this.clearTimers();
            if (this.ws) {
                this.ws.close(1000, '页面关闭');
            }
        }
    }
});