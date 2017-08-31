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
using OpenCvSharp;
using System.Threading;

namespace EDC19_host_computer
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>

    public partial class MainWindow : System.Windows.Window
    {
        SerialPort sp1 = new SerialPort();
        WriteableBitmap bmp = null;

        public MainWindow()
        {
            InitializeComponent();
            sp1_init();
            spView_init();
            cameraView_init();
        }

        public void cameraView_init()
        {
            BitmapImage bi = new BitmapImage();
            bi.BeginInit();
            Uri u = new Uri(System.Environment.CurrentDirectory + @"\lena.bmp", UriKind.RelativeOrAbsolute);
            bi.UriSource = u;
            bi.EndInit();
            imgCamera.Source = bi;
        }

        public void sp1_dataAnalyse(byte[] content)
        {

        }

        public void cameraLoop()
        {
            VideoCapture vc = new VideoCapture(CaptureDevice.DShow);
            Mat mat = new Mat();
            while (true)
            {
                vc.Read(mat);
                this.Dispatcher.Invoke(() =>
                {
                    if (bmp == null || bmp.PixelWidth != mat.Width || bmp.PixelHeight != mat.Height)
                    {
                        bmp = new WriteableBitmap(mat.Width, mat.Height, 96, 96, PixelFormats.Bgr24, null);
                        imgCamera.Source = bmp;
                    }
                    bmp.WritePixels(new Int32Rect(0, 0, mat.Width, mat.Height), mat.Data, (int)(mat.Height * mat.Step()), (int)mat.Step(), 0, 0);
                });
            }
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            imgCamera.Source = bmp;

            var thread = new Thread(cameraLoop);
            thread.Start();
        }
    }
}
