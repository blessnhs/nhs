﻿
// -------------------------------------------------
// Developed By : Ragheed Al-Tayeb
// e-Mail       : ragheedemail@gmail.com
// Date         : April 2012
// -------------------------------------------------
using System.Collections.Concurrent;
using System.IO;

namespace rtaNetworking.Streaming
{


    /// <summary>
    /// Provides a streaming server that can be used to stream any images source
    /// to any client.
    /// </summary>
    public class ImageStreamingServer : System.IDisposable
    {

        private System.Collections.Generic.List<System.Net.Sockets.Socket> _Clients;
        private System.Threading.Thread _Thread;


        public ImageStreamingServer()
        //: this(Screen.Snapshots(600, 450, true))
            
        {
            _Clients = new System.Collections.Generic.List<System.Net.Sockets.Socket>();
            _Thread = null;
            this.Interval = 50;

            Start();
        }


        public ImageStreamingServer(ConcurrentQueue<System.IO.MemoryStream> imagesSource)
        {

            _Clients = new System.Collections.Generic.List<System.Net.Sockets.Socket>();
            _Thread = null;

            this.ImagesSource = imagesSource;
            this.Interval = 50;

        }


        /// <summary>
        /// Gets or sets the source of images that will be streamed to the 
        /// any connected client.
        /// </summary>
        public ConcurrentQueue<System.IO.MemoryStream> ImagesSource = new ConcurrentQueue<System.IO.MemoryStream>();

        /// <summary>
        /// Gets or sets the interval in milliseconds (or the delay time) between 
        /// the each image and the other of the stream (the default is . 
        /// </summary>
        public int Interval { get; set; }

        /// <summary>
        /// Gets a collection of client sockets.
        /// </summary>
        public System.Collections.Generic.IEnumerable<System.Net.Sockets.Socket> Clients { get { return _Clients; } }

        /// <summary>
        /// Returns the status of the server. True means the server is currently 
        /// running and ready to serve any client requests.
        /// </summary>
        public bool IsRunning { get { return (_Thread != null && _Thread.IsAlive); } }


        /// <summary>
        /// Starts the server to accepts any new connections on the specified port.
        /// </summary>
        /// <param name="port"></param>
        public void Start(int port)
        {

            lock (this)
            {
                _Thread = new System.Threading.Thread(new System.Threading.ParameterizedThreadStart(ServerThread));
                _Thread.IsBackground = true;
                _Thread.Start(port);
            }

        }


        /// <summary>
        /// Starts the server to accepts any new connections on the default port (8080).
        /// </summary>
        public void Start()
        {
            this.Start(8080);
        }


        public void Stop()
        {

            if (this.IsRunning)
            {
                try
                {
                    _Thread.Join();
                    _Thread.Abort();
                }
                finally
                {

                    lock (_Clients)
                    {

                        foreach (var s in _Clients)
                        {
                            try
                            {
                                s.Close();
                            }
                            catch { }
                        }
                        _Clients.Clear();

                    }

                    _Thread = null;
                }
            }
        }


        /// <summary>
        /// This the main thread of the server that serves all the new 
        /// connections from clients.
        /// </summary>
        /// <param name="state"></param>
        private void ServerThread(object state)
        {
            try
            {
                System.Net.Sockets.Socket Server = new System.Net.Sockets.Socket(
                    System.Net.Sockets.AddressFamily.InterNetwork, System.Net.Sockets.SocketType.Stream, System.Net.Sockets.ProtocolType.Tcp);

                Server.Bind(new System.Net.IPEndPoint(System.Net.IPAddress.Any, (int)state));
                Server.Listen(10);

                System.Diagnostics.Debug.WriteLine(string.Format("Server started on port {0}.", state));

                foreach (System.Net.Sockets.Socket client in Server.IncommingConnectoins())
                {
                    System.Threading.ThreadPool.QueueUserWorkItem(new System.Threading.WaitCallback(ClientThread), client);
                } // Next client 

            }
            catch { }

            this.Stop();
        }


        /// <summary>
        /// Each client connection will be served by this thread.
        /// </summary>
        /// <param name="client"></param>
        private void ClientThread(object client)
        {
            System.Net.Sockets.Socket socket = (System.Net.Sockets.Socket)client;
            System.Diagnostics.Debug.WriteLine(string.Format("New client from {0}", socket.RemoteEndPoint.ToString()));


            lock (_Clients)
                _Clients.Add(socket);

            try
            {
                while(true)
                {
                    using (MjpegWriter wr = new MjpegWriter(new System.Net.Sockets.NetworkStream(socket, true)))
                    {

                        // Writes the response header to the client.
                        wr.WriteHeader();

                        MemoryStream ms;
                        if (ImagesSource.TryDequeue(out ms) == true)
                        {
                            if (ms == null)
                                continue;

                            if (this.Interval > 0)
                                System.Threading.Thread.Sleep(this.Interval);

                            wr.Write(ms);
                        }
                    }


                }
              
            }
            catch(System.Exception ex) {
                System.Console.WriteLine(ex.Message);
            }
            finally
            {
                lock (_Clients)
                    _Clients.Remove(socket);
            }
        }


        public void Dispose()
        {
            this.Stop();
        }


    }


    static class SocketExtensions
    {

        public static System.Collections.Generic.IEnumerable<System.Net.Sockets.Socket> IncommingConnectoins(this System.Net.Sockets.Socket server)
        {
            while (true)
                yield return server.Accept();
        }

    }
}
