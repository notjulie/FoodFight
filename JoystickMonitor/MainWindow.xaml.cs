using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace JoystickMonitor
{
   /// <summary>
   /// Interaction logic for MainWindow.xaml
   /// </summary>
   public partial class MainWindow : Window
   {
      private TcpClient tcpClient;
      private NetworkStream stream;

      public MainWindow()
      {
         // normal component initialization
         InitializeComponent();

         // connect
         tcpClient = new TcpClient("chuck", 4242);
         stream = tcpClient.GetStream();

         // add event handlers
         shutdownButton.Click += ShutdownButton_Click;
         getImageButton.Click += GetImageButton_Click;
      }

      private void GetImageButton_Click(object sender, RoutedEventArgs e)
      {
         // get the result... it's an ugly string that takes 6 bytes to represent a pixel,
         // but it's fine for now
         string imageString = SendCommand("getImage");

         // convert to an array
         byte[] pixelData = new byte[imageString.Length / 2];
         for (int i = 0; i < pixelData.Length; ++i)
         {
            int byteValue = 
               16 * (byte)(imageString[2 * i] - 'A') +
               (byte)(imageString[2 * i + 1] - 'A');
            pixelData[i] = (byte)byteValue;
         }

         // create the bitmap
         WriteableBitmap bitmap = new WriteableBitmap(640, 480, 96, 96, PixelFormats.Rgb24, null);
         bitmap.WritePixels(
            new Int32Rect(0, 0, 640, 480),
            pixelData,
            640 * 3,
            0
            );

         // display it
         image.Source = bitmap;
      }

      private void ShutdownButton_Click(object sender, RoutedEventArgs e)
      {
         SendCommand("shutdown");
      }

      private string SendCommand(string command)
      {
         byte[] readBuffer = new byte[1000000];

         // clear the input stream
         stream.ReadTimeout = 1;
         while (stream.DataAvailable)
            stream.Read(readBuffer, 0, readBuffer.Length);

         // convert to array of bytes
         List<byte> data = new List<byte>();
         foreach (char c in command)
            data.Add((byte)c);
         data.Add((byte)'\n');
         var dataArray = data.ToArray();

         // send
         stream.Write(dataArray, 0, dataArray.Length);

         // start a timeout timer
         StringBuilder result = new StringBuilder();
         stream.ReadTimeout = 1;
         Stopwatch timeoutTimer = new Stopwatch();
         timeoutTimer.Start();

         // read until we see the newline character
         while (timeoutTimer.ElapsedMilliseconds < 10000)
         {
            // read
            int msRemaining = 10000 - (int)timeoutTimer.ElapsedMilliseconds;
            if (msRemaining < 1)
               msRemaining = 1;
            stream.ReadTimeout = msRemaining;
            int bytesRead = stream.Read(readBuffer, 0, readBuffer.Length);
            if (bytesRead > 0)
               timeoutTimer.Restart();

            // process
            for (int i = 0; i < bytesRead; ++i)
            {
               if (readBuffer[i] == (byte)'\n')
                  return result.ToString();
               result.Append((char)readBuffer[i]);
            }
         }

         throw new Exception("Socket read timeout");
      }
   }
}
