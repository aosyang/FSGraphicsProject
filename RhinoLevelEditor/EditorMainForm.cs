using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RhinoLevelEditor
{
    public partial class EditorMainForm : Form
    {
        public EditorMainForm()
        {
            InitializeComponent();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            engineCanvas1.Shutdown();
        }

        private void engineCanvas1_Load(object sender, EventArgs e)
        {
            engineCanvas1.Initialize();
        }

        private void engineCanvas1_Resize(object sender, EventArgs e)
        {

        }
    }
}
