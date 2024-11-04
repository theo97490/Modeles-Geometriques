#include "color.h"
#include "meshcolor.h"
#include "qte.h"
#include "implicits-tree.h"
#include "ui_interface.h"
#include <cmath>
#include <tp_math.h>

MainWindow::MainWindow() : QMainWindow(), uiw(new Ui::Assets)
{
	// Chargement de l'interface
    uiw->setupUi(this);

	// Chargement du GLWidget
	meshWidget = new MeshWidget;
	QGridLayout* GLlayout = new QGridLayout;
	GLlayout->addWidget(meshWidget, 0, 0);
	GLlayout->setContentsMargins(0, 0, 0, 0);
    uiw->widget_GL->setLayout(GLlayout);

	// Creation des connect
	CreateActions();

	meshWidget->SetCamera(Camera(Vector(10, 0, 0), Vector(0.0, 0.0, 0.0)));
}

MainWindow::~MainWindow()
{
	delete meshWidget;
}

void MainWindow::CreateActions()
{
	// Buttons
  connect(uiw->bezier, SIGNAL(clicked()), this, SLOT(BezierExample()));
  connect(uiw->revolution, SIGNAL(clicked()), this, SLOT(RevolutionExample()));
  connect(uiw->implicit, SIGNAL(clicked()), this, SLOT(ImplicitExampleA()));
  connect(uiw->implicit_2, SIGNAL(clicked()), this, SLOT(ImplicitExampleB()));
  connect(uiw->implicit_3, SIGNAL(clicked()), this, SLOT(ImplicitExampleC()));
  connect(uiw->implicit_4, SIGNAL(clicked()), this, SLOT(ImplicitExampleD()));
  connect(uiw->resetcameraButton, SIGNAL(clicked()), this, SLOT(ResetCamera()));
  connect(uiw->wireframe, SIGNAL(clicked()), this, SLOT(UpdateMaterial()));
  connect(uiw->radioShadingButton_1, SIGNAL(clicked()), this, SLOT(UpdateMaterial()));
  connect(uiw->radioShadingButton_2, SIGNAL(clicked()), this, SLOT(UpdateMaterial()));

	// Widget edition
	connect(meshWidget, SIGNAL(_signalEditSceneLeft(const Ray&)), this, SLOT(editingSceneLeft(const Ray&)));
	connect(meshWidget, SIGNAL(_signalEditSceneRight(const Ray&)), this, SLOT(editingSceneRight(const Ray&)));
}

void MainWindow::editingSceneLeft(const Ray&)
{
}

void MainWindow::editingSceneRight(const Ray&)
{
}

void MainWindow::BezierExample()
{
  meshWidget->ClearAll();

  // Bezier 1
  {
    BezierSurface b(3,3, std::vector{
 	  Vector{0,0,0}, Vector{0,0.5,0.5}, Vector{0,1,-0.5},
      Vector{1,0,0}, Vector{1,0.5,-0.5}, Vector{0,1,0},
      Vector{2,0,0.5}, Vector{2,0.5,0}, Vector{2,1,0}
  	});

   	meshWidget->AddMesh("1", mesh_bezier_surface(b, 100, 100));
  }

  // Bezier 2
  {
    BezierSurface b(4,4, std::vector{
      Vector{-5,-5,2}, Vector{-5,-2,-5}, Vector{-5,2,-5}, Vector{-5,5,2},
      Vector{-2,-10,5}, Vector{-2,-2,2}, Vector{-2,2,2}, Vector{-2,10,2},
      Vector{2,-10,5}, Vector{2,-2,2}, Vector{2,2,2}, Vector{2,-5,2},
      Vector{5,-5,2}, Vector{5,-2,5}, Vector{5,2,5}, Vector{5,5,2},
   	});

  	meshWidget->AddMesh("2", mesh_bezier_surface(b, 100, 100));
  }

  // Bezier 3
  {
    BezierSurface b(4,4, std::vector{
      Vector{-10,-10,5}, Vector{-10,-5,2}, Vector{-10,5,-2}, Vector{-10,10,-2},
      Vector{-5,-10,2}, Vector{-5,-5,-10}, Vector{-5,5,-10}, Vector{-5,10,-2},
      Vector{5,-10,-2}, Vector{5,-5,-10}, Vector{5,5,-10}, Vector{5,10,2},
      Vector{10,-10,-2}, Vector{10,-5,-2}, Vector{10,5,2}, Vector{10,10,5},
   	});

   	meshWidget->AddMesh("3", mesh_bezier_surface(b, 100, 100));
  }


	UpdateMaterial();
}

void MainWindow::BezierExample2()
{
  meshWidget->ClearAll();

  // a, b 2d vectors
  const auto make_default = [](size_t sx, size_t sy, Vector a, Vector b) {
    auto bezier = BezierSurface(sx,sy, std::vector<Vector>(sx*sy));
    Vector delta = b - a;
    delta[0] /= sx;
    delta[1] /= sy;
    for (int x = 0; x < sx; x++){
      for (int y = 0; y < sy; y++){
        bezier.control(x, y) = a + Vector{ x * delta[0], y * delta[1], 0 }; 
      }
    }
    return bezier;
  };

  int s = 20;
  auto b = make_default(s, s, Vector{-5, -5, 0}, Vector{5, 5, 0});
  b.control(s/2, s/2) = Vector{0,0,100};

  meshWidget->AddMesh("3", mesh_bezier_surface(b, 100, 100));
  // meshWidget->AddMesh("4", mesh_bezier_surface(b, 100, 100));
  // meshWidget->AddMesh("5", mesh_bezier_surface(b, 100, 100));

	UpdateMaterial();
}

void MainWindow::RevolutionExample()
{
	meshWidget->ClearAll();

  // Revolution 1
  {
   	BezierCurve curve(std::vector{
		  Vector(0,0,0), Vector(10,-1,0), Vector(10,20,0), Vector(-10,10,-10), Vector(0, 0,-10),
		});

  	ExtrusionSurface ext(&curve, [&](double rad){
  		double hdiv =  M_PI / 10;
  		double v = std::fmod(rad, 2*hdiv);
  	  return 0.25 + sin(v/2*M_PI);
  	});

   meshWidget->AddMesh("1", mesh_extrusion_surface(ext, 200, 200));
  }

  // Revolution 2
  {
   	BezierCurve curve(std::vector{
		  Vector(0,0,10), Vector(0,2,5), Vector(0,5,0), Vector(0,5,-5), Vector(0, 5, -10)
		});

  	ExtrusionSurface ext(&curve, [&](double rad){
      return 1;
  	});

   meshWidget->AddMesh("2", mesh_extrusion_surface(ext, 50, 50));
  }

  // Revolution 3
  {
  //  	BezierCurve curve(std::vector{
		//   Vector(0,0,0), Vector(10,-1,0), Vector(10,20,0), Vector(-20,40,-40),
		// });

  // 	ExtrusionSurface ext(&curve, [&](double rad){
  // 		double hdiv =  M_PI / 10;
  // 		double v = std::fmod(rad, 2*hdiv);
  // 	  // return v < hdiv
  // 			// ? 0.25 + v/hdiv
  // 			// : 0.25 + 2 - v/hdiv
  // 			// ;

  // 	  return 0.25 + sin(v/2*M_PI)
  // 			;
  // 	});

  //  meshWidget->AddMesh("3", mesh_extrusion_surface(ext, 50, 50));
  }

	UpdateMaterial();
}

void MainWindow::ImplicitExampleA() {
  Mesh m;
  auto b = ImplicitTree::InigoBox(Vector{5,0,0}, Vector{8,8,8});
  auto tore = ImplicitTree::InigoTore(Vector::Null, Vector{10, 3, 0});
  auto i = ImplicitTree::Diff(b, tore);
  ImplicitTree::Tree(&i).Polygonize(400, m, Box(20), 0.001);
  meshWidget->ClearAll();
  meshWidget->AddMesh("1", m);
  UpdateMaterial();
}

void MainWindow::ImplicitExampleB() {
  Mesh m;
  double htsize = 0.25;
  double htheight = 3;
  double d = 5;

  auto t0 = ImplicitTree::Capsule(Vector{-d, -d, htheight}, Vector::Z, htheight, htsize);
  auto t1 = ImplicitTree::Capsule(Vector{d, -d, htheight}, Vector::Z, htheight, htsize);
  auto t2 = ImplicitTree::Capsule(Vector{-d, d, htheight}, Vector::Z, htheight, htsize);
  auto t3 = ImplicitTree::Capsule(Vector{d, d, htheight}, Vector::Z, htheight, htsize);

  auto tt0 = ImplicitTree::Union(t0, t1);
  auto tt1 = ImplicitTree::Union(t2, t3);
  auto tunion = ImplicitTree::Union(tt0, tt1);

  auto box = ImplicitTree::InigoBox(Vector{0,0, htheight}, Vector{5,5,0.1});
  auto box2 = ImplicitTree::InigoBox(Vector{0,0, 2*htheight}, Vector{5,5,0.1});

  auto bunion = ImplicitTree::Union(box, box2);
  auto implicit = ImplicitTree::Blend(bunion, tunion, 10);

  auto a = ImplicitTree::Sphere(Vector::Null, 3);
  auto b = ImplicitTree::Sphere(Vector{0,0,0.25}, 3);
  auto c = ImplicitTree::Sphere(Vector{-0.25,0.1,-1.2}, 1);
  auto cc = ImplicitTree::Diff(a,b);
  auto aze = ImplicitTree::Scale(&cc, Vector{1.1, 1.3, 0.7});
  auto saladier = ImplicitTree::Union(c,aze);

  auto st = ImplicitTree::Translate(&saladier, Vector{0,0,htheight*2 + 2});
  auto implicit2 = ImplicitTree::Union(implicit, st);

  ImplicitTree::Tree(&implicit2).Polygonize(500, m, Box(30), 0.001);
  meshWidget->ClearAll();
  meshWidget->AddMesh("1", m);
  UpdateMaterial();
}

void MainWindow::ImplicitExampleC() {
  Mesh m;
  double htsize = 0.25;
  double htheight = 3;
  double d = 5;

  auto a = ImplicitTree::InigoBox(Vector::Null, Vector(5));
  auto b = ImplicitTree::InigoBox(Vector{10,10,10}, Vector(5));
  auto c = ImplicitTree::Sphere(Vector(18), 5);
  auto dd = ImplicitTree::Sphere(Vector(24), 5);

  auto aa = ImplicitTree::Blend(a, b, 30);
  auto bb = ImplicitTree::Blend(aa, c, 100);
  auto p = ImplicitTree::Blend(bb, dd, 50);

  ImplicitTree::Tree(&p).Polygonize(500, m, Box(100), 0.001);
  meshWidget->ClearAll();
  meshWidget->AddMesh("1", m);
  UpdateMaterial();
}

void MainWindow::ImplicitExampleD() {
    Mesh m;
    auto torus = ImplicitTree::InigoTore(Vector{0,0,0}, Vector{10,5,0});
    auto repl = ImplicitTree::Replicate(&torus, Vector{20,20,20});
    ImplicitTree::Tree(&repl).Polygonize(400, m, Box(300), 0.001);
    meshWidget->ClearAll();
    meshWidget->AddMesh("1", m);
    UpdateMaterial();
}

void MainWindow::UpdateGeometry()
{
	meshWidget->ClearAll();
	meshWidget->AddMesh("BoxMesh", meshColor);

    uiw->lineEdit->setText(QString::number(meshColor.Vertexes()));
    uiw->lineEdit_2->setText(QString::number(meshColor.Triangles()));

	UpdateMaterial();
}

void MainWindow::UpdateMaterial()
{
    meshWidget->UseWireframeGlobal(uiw->wireframe->isChecked());

    if (uiw->radioShadingButton_1->isChecked())
		meshWidget->SetMaterialGlobal(MeshMaterial::Normal);
	  else
		meshWidget->SetMaterialGlobal(MeshMaterial::Color);
}

void MainWindow::ResetCamera()
{
	meshWidget->SetCamera(Camera(Vector(-10.0), Vector(0.0)));
}
