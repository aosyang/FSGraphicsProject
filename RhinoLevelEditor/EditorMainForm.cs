using ManagedInterface;
using System;
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
            engineCanvas1.SetLogLabel(ref toolStripStatusLabel1);
            listMesh.DataSource = engineCanvas1.RhinoEngine.GetMeshNameList();
        }

        private void btnAddMesh_Click(object sender, EventArgs e)
        {
            engineCanvas1.RhinoEngine.UpdatePreviewMesh(listMesh.SelectedItem.ToString(), false);
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

        private void listMesh_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            int index = this.listMesh.IndexFromPoint(e.Location);
            if (index != System.Windows.Forms.ListBox.NoMatches)
            {
                engineCanvas1.RhinoEngine.UpdatePreviewMesh(listMesh.Items[index].ToString(), false);
            }
        }

        private void UpdatePropertyGrid()
        {
            IManagedSceneObject go = engineCanvas1.RhinoEngine.GetSelection();
            if (go.IsValid())
                propertyGrid1.SelectedObject = go;
            else
                propertyGrid1.SelectedObject = null;
        }

        private void engineCanvas1_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Left)
            {
                EngineCanvas canvas = (EngineCanvas)sender;
                float x = (float)e.X / canvas.Width, y = (float)e.Y / canvas.Height;
                toolStripStatusLabel1.Text = x + " " + y;
                //engineCanvas1.RhinoEngine.RunScreenToCameraRayPicking(x, y);

                UpdatePropertyGrid();
            }
        }

        private void deleteSelectionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            engineCanvas1.RhinoEngine.DeleteSelection();
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Filter = "Map files (*.rmap)|*.rmap";

            if (dlg.ShowDialog() == DialogResult.OK)
            {
                engineCanvas1.RhinoEngine.SaveScene(dlg.FileName);
            }
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Filter = "Map files (*.rmap)|*.rmap";

            if (dlg.ShowDialog() == DialogResult.OK)
            {
                engineCanvas1.RhinoEngine.LoadScene(dlg.FileName);
                UpdatePropertyGrid();
            }
        }

        private void saveMaterialToolStripMenuItem_Click(object sender, EventArgs e)
        {
            engineCanvas1.RhinoEngine.SaveMeshMaterialFromSelection();
        }

        private void exportAnimToolStripMenuItem_Click(object sender, EventArgs e)
        {
            engineCanvas1.RhinoEngine.ExportAllAnimationsToBinaryFiles();
        }

        private void btnReplaceMesh_Click(object sender, EventArgs e)
        {
            engineCanvas1.RhinoEngine.UpdatePreviewMesh(listMesh.SelectedItem.ToString(), true);
            UpdatePropertyGrid();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
