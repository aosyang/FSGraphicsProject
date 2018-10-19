//=============================================================================
// EditorMainForm.cs by Shiyang Ao, 2018 All Rights Reserved.
//
//
//=============================================================================
using ManagedInterface;
using System;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace RhinoLevelEditor
{
    public partial class EditorMainForm : Form
    {
        bool[] KeyState = new bool[255];
        const string FormTitle = "Rhino Level Editor";
        string OpenedMapPath;
        bool bUpdatingSceneObjectsList = false;

        const int ThumbnailWidth = 128;
        const int ThumbnailHeight = 128;

        public EditorMainForm()
        {
            InitializeComponent();

            RefreshFormTitle();
        }

        /// <summary>
        /// Refresh the title of window form
        /// </summary>
        private void RefreshFormTitle()
        {
            string MapName = Path.GetFileName(OpenedMapPath);
            if (MapName == null || MapName == "")
            {
                Text = FormTitle + " - Untitled";
            }
            else
            {
                Text = FormTitle + " - " + MapName;
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            engineCanvas1.Shutdown();
        }

        private void engineCanvas1_Load(object sender, EventArgs e)
        {
            engineCanvas1.Initialize();
            engineCanvas1.SetLogLabel(ref toolStripStatusLabel1);
            listBox_MeshAssets.DataSource = engineCanvas1.RhinoEngine.GetMeshNameList();

            int Index = 0;
            foreach (string MeshAssetPath in engineCanvas1.RhinoEngine.GetMeshNameList())
            {
                string FileName = Path.GetFileName(MeshAssetPath);
                ListViewItem item = new ListViewItem(FileName, Index);

                EngineAssetData AssetData = new EngineAssetData();
                AssetData.Path = MeshAssetPath;

                item.Tag = AssetData;
                listView_AssetView.Items.Add(item);

                Index++;
            }

            // Register async resource loaded event
            AsyncResourceLoadedHandler ResourceLoadedEvent = new AsyncResourceLoadedHandler(OnAsyncResourceLoaded);
            engineCanvas1.RhinoEngine.SetAsyncResourceLoadedHandler(ResourceLoadedEvent);

            UpdateAssetListViewThumbnails();

            RefreshSceneObjectsListBox();
        }

        private void btnAddMesh_Click(object sender, EventArgs e)
        {
            string MeshAssetPath = listBox_MeshAssets.SelectedItem.ToString();
            engineCanvas1.RhinoEngine.AddMeshObjectToScene(MeshAssetPath);
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
            int index = listBox_MeshAssets.IndexFromPoint(e.Location);
            if (index != ListBox.NoMatches)
            {
                string MeshAssetPath = listBox_MeshAssets.Items[index].ToString();
                engineCanvas1.RhinoEngine.AddMeshObjectToScene(MeshAssetPath);
                RefreshSceneObjectsListBox();
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

        /// <summary>
        /// Refresh scene object list box content with active scene state
        /// </summary>
        private void RefreshSceneObjectsListBox()
        {
            bUpdatingSceneObjectsList = true;

            sceneObjectsListBox.DataSource = engineCanvas1.RhinoEngine.GetSceneObjectsList();
            sceneObjectsListBox.DisplayMember = "DisplayName";
            sceneObjectsListBox.Refresh();

            IManagedSceneObject Selection = engineCanvas1.RhinoEngine.GetSelection();
            if (Selection.IsValid())
            {
                sceneObjectsListBox.SelectedItem = Selection;
            }
            else
            {
                sceneObjectsListBox.SelectedItem = null;
            }

            bUpdatingSceneObjectsList = false;
        }

        /// <summary>
        /// Update thumbnails of all assets in asset list view
        /// </summary>
        private void UpdateAssetListViewThumbnails()
        {
            ImageList LargeImageList = new ImageList();
            LargeImageList.ImageSize = new Size(ThumbnailWidth, ThumbnailHeight);
            LargeImageList.ColorDepth = ColorDepth.Depth32Bit;

            foreach (ListViewItem Item in listView_AssetView.Items)
            {
                if (Item != null)
                {
                    EngineAssetData AssetData = Item.Tag as EngineAssetData;
                    if (AssetData != null)
                    {
                        string AssetPath = AssetData.Path;

                        if (engineCanvas1.RhinoEngine.IsMeshAssetReady(AssetPath))
                        {
                            Bitmap Image = engineCanvas1.RhinoEngine.GenerateMeshThumbnailBitmap(AssetPath, ThumbnailWidth, ThumbnailHeight);
                            LargeImageList.Images.Add(Image);
                        }
                        else
                        {
                            // Add place holder for deferred-updating thumbnails
                            Bitmap Image = new Bitmap(ThumbnailWidth, ThumbnailHeight);
                            LargeImageList.Images.Add(Image);
                        }
                    }
                }
            }

            listView_AssetView.LargeImageList = LargeImageList;
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
                RefreshSceneObjectsListBox();
            }
        }

        private void deleteSelectionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (engineCanvas1.Focused)
            {
                if (engineCanvas1.RhinoEngine.DeleteSelection())
                {
                    UpdatePropertyGrid();
                    RefreshSceneObjectsListBox();
                }
            }
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Filter = "Map files (*.rmap)|*.rmap";

            if (dlg.ShowDialog() == DialogResult.OK)
            {
                OpenedMapPath = dlg.FileName;
                engineCanvas1.RhinoEngine.LoadScene(dlg.FileName);
                RefreshFormTitle();
                UpdatePropertyGrid();
                RefreshSceneObjectsListBox();
            }
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (OpenedMapPath != null && OpenedMapPath != "")
            {
                engineCanvas1.RhinoEngine.SaveScene(OpenedMapPath);
            }
            else
            {
                saveAsToolStripMenuItem_Click(sender, e);
            }
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Filter = "Map files (*.rmap)|*.rmap";

            if (dlg.ShowDialog() == DialogResult.OK)
            {
                OpenedMapPath = dlg.FileName;
                engineCanvas1.RhinoEngine.SaveScene(dlg.FileName);
                RefreshFormTitle();
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
            string MeshAssetPath = listBox_MeshAssets.SelectedItem.ToString();
            engineCanvas1.RhinoEngine.ReplaceMeshAssetForSelection(MeshAssetPath);
            UpdatePropertyGrid();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void sceneObjectsListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Avoid running the scene object selection during refreshing list box
            if (!bUpdatingSceneObjectsList)
            {
                ListBox listBox = (ListBox)sender;
                engineCanvas1.RhinoEngine.SetSelection((IManagedSceneObject)listBox.SelectedItem);
                UpdatePropertyGrid();
            }
        }

        private void refreshAssetsPreviewsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            UpdateAssetListViewThumbnails();
        }

        private void OnAsyncResourceLoaded(string ResourceName)
        {
            if (Path.GetExtension(ResourceName).ToLower() == ".fbx")
            {
                int Index = 0;

                foreach (ListViewItem Item in listView_AssetView.Items)
                {
                    if (Item != null)
                    {
                        EngineAssetData AssetData = Item.Tag as EngineAssetData;

                        if (AssetData.Path == ResourceName)
                        {
                            Bitmap Image = engineCanvas1.RhinoEngine.GenerateMeshThumbnailBitmap(ResourceName, ThumbnailWidth, ThumbnailHeight);
                            listView_AssetView.LargeImageList.Images[Index] = Image;

                            // Refresh a single item with thumbnail
                            listView_AssetView.RedrawItems(Index, Index, false);

                            break;
                        }
                    }

                    Index++;
                }
            }
        }

        private void listView_AssetView_DoubleClick(object sender, EventArgs e)
        {
            if (listView_AssetView.SelectedItems.Count > 0)
            {
                ListViewItem Item = listView_AssetView.SelectedItems[0];
                EngineAssetData AssetData = Item.Tag as EngineAssetData;
                if (AssetData != null)
                {
                    engineCanvas1.RhinoEngine.AddMeshObjectToScene(AssetData.Path);
                }
            }
        }
    }
}
