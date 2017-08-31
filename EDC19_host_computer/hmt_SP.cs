using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO.Ports;
using System.Linq;
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

namespace EDC19_host_computer
{
    public partial class MainWindow : Window
    {
        Dictionary<int, string> spBaud = new Dictionary<int, string>
        {
            {1, "9600" },
            {2, "115200"},
        };
        Dictionary<int, string> spNames = new Dictionary<int, string>
        {

        };

        private void spView_init()
        {
            this.cbSpBaudRate.ItemsSource = this.spBaud;
            this.cbSpBaudRate.SelectedValuePath = "Key";
            this.cbSpBaudRate.DisplayMemberPath = "Value";
            this.cbSpBaudRate.SelectedIndex = 0;

            this.cbSpId.ItemsSource = this.spNames;
            this.cbSpId.SelectedValuePath = "Key";
            this.cbSpId.DisplayMemberPath = "Value";
            string[] names = SerialPort.GetPortNames();
            if (names.Length != 0)
            {
                int i = 0;
                foreach (string str in names)
                {
                    spNames.Add(++i, str);
                }
                this.cbSpId.Items.Refresh();
                this.cbSpId.SelectedIndex = 0;
            }

        }

        private void sp1_init()
        {
            sp1.BaudRate = 9600;
            sp1.DataBits = 8;
            sp1.StopBits = StopBits.One;
            sp1.Parity = Parity.None;
            sp1.PortName = "COM5";

            sp1.DataReceived += new SerialDataReceivedEventHandler(sp1_DataReceived);
            sp1.ReceivedBytesThreshold = 1;
            sp1.DtrEnable = true;
            sp1.RtsEnable = true;
            sp1.ReadTimeout = 1000;
            sp1.WriteTimeout = 50;
            sp1.Close();
        }

        public void sp1_SendData(byte[] content)
        {
            if (sp1.IsOpen)
            {
                sp1.Write(content, 0, content.Length);
            }
        }

        public void sp1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            if (sp1.IsOpen)
            {
                byte[] content = new byte[sp1.BytesToRead];
                sp1.Read(content, 0, content.Length);
                sp1.DiscardInBuffer();
                sp1_dataAnalyse(content);
            }
        }

        private void onClick_SpStart(object sender, RoutedEventArgs e)
        {
            if (!sp1.IsOpen)
            {
                if (this.cbSpBaudRate.Text == "")
                {
                    MessageBox.Show("Please choose baudrate!");
                }
                else if (this.cbSpId.Text == "")
                {
                    MessageBox.Show("Please right port!");
                }
                else
                {
                    sp1.BaudRate = int.Parse(this.cbSpBaudRate.Text);
                    sp1.PortName = this.cbSpId.Text;
                    sp1.Open();
                    this.btnSpStart.Content = "关闭串口";
                    MessageBox.Show("sp1 is opend");
                }
            }
            else
            {
                sp1.Close();
                this.btnSpStart.Content = "打开串口";
                MessageBox.Show("sp1 is closed");
            }
        }

        private void onClick_SpSend(object sender, RoutedEventArgs e)
        {
            sp1.WriteLine("Test");
        }

        private void onDDOpen_SpBaudRate(object sender, EventArgs e)
        {
        }

        private void onDDOpen_SpId(object sender, EventArgs e)
        {
            spNames.Clear();
            int i = 0;
            foreach (string str in SerialPort.GetPortNames())
            {
                spNames.Add(++i, str);
            }
            this.cbSpId.Items.Refresh();
        }
    }
}