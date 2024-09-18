using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace CCA.Network
{
    public class AsyncSocket
    {
        private static readonly int BUFFER_SIZE = 8192;
        private byte[] _buffer = new byte[BUFFER_SIZE];
        private SocketAsyncEventArgs _readEventArg = null;
        private SocketAsyncEventArgs _writeEventArg = null;

        public string GetIPAddress(string hostname)
        {
            IPHostEntry host;
            host = Dns.GetHostEntry(hostname);

            foreach (IPAddress ip in host.AddressList)
            {
                if (ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)
                {
                    //System.Diagnostics.Debug.WriteLine("LocalIPadress: " + ip);
                    return ip.ToString();
                }
            }
            return string.Empty;
        }

        #region Constructors
        public AsyncSocket()
        {

            Device = null;

            _readEventArg = new SocketAsyncEventArgs();
            _readEventArg.SetBuffer(_buffer, 0, BUFFER_SIZE);
            _readEventArg.Completed += new EventHandler<SocketAsyncEventArgs>(OnReceiveCompleted);

            _writeEventArg = new SocketAsyncEventArgs();
            _writeEventArg.Completed += new EventHandler<SocketAsyncEventArgs>(OnSendCompleted);
        }
        public void Close()
        {
            if (Device != null)
            {
                Device.Close();
                Device = null;
            }
            if (_readEventArg != null)
            {
                _readEventArg = null;
            }
            if (_writeEventArg != null)
            {
                _writeEventArg = null;
            }
        }
        #endregion

        #region Properties
        public bool IsConnected
        {
            get
            {
                if (Device == null)
                {
                    return false;
                }

                return Device.Connected;
            }
        }
        public IPEndPoint EP
        {
            get
            {
                if (Device == null)
                {
                    return null;
                }

                return Device.RemoteEndPoint as IPEndPoint;
            }
        }
        private Socket Device
        {
            get;
            set;
        }
        #endregion

        #region Connect Functions
        //public override bool Connect(IPEndPoint inEndPoint)
        public bool Connect(IPEndPoint inEndPoint)
        {
            try
            {
                if (IsConnected)
                {
                    return false;
                }

                if (Device == null)
                {
                    Device = new Socket(inEndPoint.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
                }

                Device.Connect(inEndPoint);

                IssueReceive();
            }
            catch (SocketException e)
            {
                Close();
                return false;
            }

            return true;
        }
        #endregion

        #region Override Functions
        public void IssueSend(MemoryStream inStream)
        {
            lock (this)
            {

                try
                {
                    _writeEventArg.SetBuffer(inStream.GetBuffer(), 0, (int)inStream.Length);
                    _writeEventArg.UserToken = inStream;

                    if (Device.SendAsync(_writeEventArg) == false)
                    {
                        ProcessSend(_writeEventArg);
                    }
                }
                catch (SocketException e)
                {
                    Close();
                }
                finally
                {

                }
            }
        }

        public void IssueSendSync(MemoryStream inStream)
        {
            lock (this)
            {

                try
                {
                    Device.Send(inStream.GetBuffer(), 0, (int)inStream.Length, SocketFlags.None);
                }
                catch (SocketException e)
                {
                    SetLastError(e);
                    Close();
                }
                finally
                {

                }
            }
        }
        protected void IssueReceive()
        {
            try
            {
                if (Device.ReceiveAsync(_readEventArg) == false)
                {
                    ProcessReceive(_readEventArg);
                }
            }
            catch (SocketException e)
            {
                SetLastError(e);
                Close();
            }
        }
        public void OnPassesStream(int inId, byte[] inBuf)
        {

        }

        #endregion

        #region IO Functions
        private void ProcessSend(SocketAsyncEventArgs inEventArgs)
        {
            try
            {
                if (inEventArgs.SocketError == SocketError.Success)
                {
                    var s = (MemoryStream)inEventArgs.UserToken;

                    int bytesSent = inEventArgs.BytesTransferred;


                    if (bytesSent != (int)s.Length)
                    {
                        _writeEventArg.SetBuffer(
                            s.GetBuffer(),
                            bytesSent,
                            (int)s.Length - bytesSent
                            );

                        if (Device.SendAsync(_writeEventArg) == false)
                        {
                            ProcessSend(_writeEventArg);
                        }
                    }

                    _writeEventArg.UserToken = null;
                }
                else
                {
                    Close();
                }
            }
            catch (SocketException e)
            {
                SetLastError(e);
                Close();
            }
        }

        public void SetLastError(Exception inException)
        {

        }

        private void ProcessReceive(SocketAsyncEventArgs inEventArgs)
        {
            if (inEventArgs.SocketError == SocketError.Success)
            {
                int bytesRead = inEventArgs.BytesTransferred;

                if (bytesRead > 0)
                {
                    //     Receiver.ReceivedData(this, new MemoryStream(_buffer, 0, bytesRead));

                    IssueReceive();
                }
                else
                {
                    Close();
                }
            }
            else
            {
                Close();
            }
        }
        private void OnSendCompleted(object inSender, SocketAsyncEventArgs inEventArgs)
        {
            if (inEventArgs.LastOperation != SocketAsyncOperation.Send)
            {
                throw new ArgumentException("The last operation completed on the socket was not a send");
            }

            ProcessSend(inEventArgs);
        }
        private void OnReceiveCompleted(object inSender, SocketAsyncEventArgs inEventArgs)
        {
            if (inEventArgs.LastOperation != SocketAsyncOperation.Receive)
            {
                throw new ArgumentException("The last operation completed on the socket was not a receive");
            }
            ProcessReceive(inEventArgs);
        }
        #endregion
    }
}
