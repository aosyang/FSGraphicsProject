using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RhinoLevelEditor
{
    public partial class EditorMainForm : Form
    {
        bool[] KeyState = new bool[255];

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
            listMesh.DataSource = engineCanvas1.RhinoEngine.GetMeshNameList();
        }

        private void btnAddMesh_Click(object sender, EventArgs e)
        {
            engineCanvas1.RhinoEngine.UpdatePreviewMesh(listMesh.SelectedItem.ToString());
        }

        private void engineCanvas1_KeyDown(object sender, KeyEventArgs e)
        {
            if (!KeyState[e.KeyValue])
            {
                KeyState[e.KeyValue] = true;
                engineCanvas1.RhinoEngine.OnKeyDown(e.KeyValue);
            }
        }

        private void engineCanvas1_KeyUp(object sender, KeyEventArgs e)
        {
            if (KeyState[e.KeyValue])
            {
                KeyState[e.KeyValue] = false;
                engineCanvas1.RhinoEngine.OnKeyUp(e.KeyValue);
            }
        }
    }
}
