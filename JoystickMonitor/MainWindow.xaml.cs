using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Text;
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

      public MainWindow()
      {
         // normal component initialization
         InitializeComponent();

         // connect
         tcpClient = new TcpClient("chuck", 4242);

         // add event handlers
         shutdownButton.Click += ShutdownButton_Click;
      }

      private void ShutdownButton_Click(object sender, RoutedEventArgs e)
      {
         SendCommand("shutdown");
      }

      private void SendCommand(string command)
      {
         List<byte> data = new List<byte>();
         foreach (char c in command)
            data.Add((byte)c);
         data.Add((byte)'\n');
         var dataArray = data.ToArray();

         var stream = tcpClient.GetStream();
         stream.Write(dataArray, 0, dataArray.Length);
      }
   }
}
