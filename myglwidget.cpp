#include "myglwidget.h"
#include <QKeyEvent>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MyGLWidget::MyGLWidget(QWidget *parent) : QOpenGLWidget(parent), program(nullptr) {
    camaraGeneral = true;
    int mapa[N][M] = {
        {1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < M; ++j) {
            laberint[i][j] = mapa[i][j];
            if (mapa[i][j] == 2) { posMorty[0] = i; posMorty[1] = j; }
            if (mapa[i][j] == 3) { posFantasma[0] = i; posFantasma[1] = j; }
            if (mapa[i][j] == 4) { posSortida[0] = i; posSortida[1] = j; }
        }
}

MyGLWidget::~MyGLWidget() {
    makeCurrent();
    glDeleteVertexArrays(1, &VAO_Cub);
    glDeleteVertexArrays(1, &VAO_Morty);
    glDeleteVertexArrays(1, &VAO_Fantasma);
    glDeleteVertexArrays(1, &VAO_Moneda);
    glDeleteVertexArrays(1, &VAO_Torre);
}

void MyGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.5, 0.7, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    program = new QOpenGLShaderProgram(this);
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, "basicShader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, "basicShader.frag");
    program->link();
    program->bind();

    vertexLoc = glGetAttribLocation(program->programId(), "vertex");
    normalLoc = glGetAttribLocation(program->programId(), "normal");
    matdiffLoc = glGetAttribLocation(program->programId(), "matdiff");
    transLoc = glGetUniformLocation(program->programId(), "TG");
    projLoc = glGetUniformLocation(program->programId(), "proj");
    viewLoc = glGetUniformLocation(program->programId(), "view");

    creaBuffersCub();

    // Carga de archivos (Asegúrate de que los .obj estén en la carpeta del proyecto)
    morty.load("Morty.obj"); creaBuffersModel(morty, VAO_Morty);
    fantasma.load("Fantasma.obj"); creaBuffersModel(fantasma, VAO_Fantasma);
    moneda.load("Coin.obj"); creaBuffersModel(moneda, VAO_Moneda);
    torre.load("tower.obj"); creaBuffersModel(torre, VAO_Torre);

    ra = 1.0f; fov = M_PI / 3.0f; znear = 0.1f; zfar = 100.0f;
}

void MyGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    projectTransform();
    viewTransform();

    // 1. Suelo y Paredes
    glBindVertexArray(VAO_Cub);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            modelTransformSuelo(i, j); glDrawArrays(GL_TRIANGLES, 0, 36);
            if (laberint[i][j] == 1) {
                modelTransformPared(i, j); glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
    }

    // 2. Personajes (Alturas e Inicio según PDF Iteración 1)
    // Dibujar Personajes y Salida
    glBindVertexArray(VAO_Morty);
    modelTransformMorty();
    glDrawArrays(GL_TRIANGLES, 0, morty.faces().size() * 3);

    glBindVertexArray(VAO_Fantasma);
    modelTransformFantasma();
    glDrawArrays(GL_TRIANGLES, 0, fantasma.faces().size() * 3);

    glBindVertexArray(VAO_Torre);
    modelTransformTorre();
    glDrawArrays(GL_TRIANGLES, 0, torre.faces().size() * 3);

    // Dibujar las 10 monedas
    glBindVertexArray(VAO_Moneda);
    for (int i = 0; i < 10; ++i) {
        modelTransformMoneda(posMonedes[i].x, posMonedes[i].z);
        glDrawArrays(GL_TRIANGLES, 0, moneda.faces().size() * 3);
    }
}

void MyGLWidget::creaBuffersModel(Model &m, GLuint &vao) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint vbo[3]; // AHORA SON 3: Vértices, Normales y Color (Matdiff)
    glGenBuffers(3, vbo);

    // Vértices
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*m.faces().size()*9, m.VBO_vertices(), GL_STATIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    // Normales
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*m.faces().size()*9, m.VBO_normals(), GL_STATIC_DRAW);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(normalLoc);

    // Color (Matdiff)
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*m.faces().size()*9, m.VBO_matdiff(), GL_STATIC_DRAW);
    glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matdiffLoc);

    glBindVertexArray(0);
}

void MyGLWidget::viewTransform() {
    glm::mat4 View;
    if (camaraGeneral) {
        // Miramos al centro exacto del laberinto (X=2, Z=7) desde arriba y atrás
        View = glm::lookAt(glm::vec3(2, 12, 15), glm::vec3(2, 0, 7), glm::vec3(0, 1, 0));
    } else {
        // Cámara en 1ª Persona (Ojos de Morty)
        View = glm::lookAt(glm::vec3(posMorty[0], 1.5f, posMorty[1]),
                           glm::vec3(posMorty[0], 1.5f, posMorty[1] + 1), // Mira hacia adelante
                           glm::vec3(0, 1, 0));
    }
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &View[0][0]);
}

void MyGLWidget::projectTransform() {
    float f = camaraGeneral ? fov : M_PI/2.0f; // FOV 90 para Morty
    glm::mat4 Proj = glm::perspective(f, ra, znear, zfar);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::modelTransformSuelo(int x, int z) {
    glm::mat4 TG = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0, z));
    TG = glm::scale(TG, glm::vec3(1.0, 0.1, 1.0));
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::modelTransformPared(int x, int z) {
    glm::mat4 TG = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0, z));
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::modelTransformMorty() {
    glm::mat4 TG(1.0f);
    TG = glm::translate(TG, glm::vec3(posMorty[0], 0, posMorty[1]));
    TG = glm::scale(TG, glm::vec3(1.5f / 312.3f)); // Alçada 1.5 / Orig 312.3
    TG = glm::translate(TG, -glm::vec3(100, -213, -6)); // Centro base Morty
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::modelTransformFantasma() {
    glm::mat4 TG(1.0f);
    TG = glm::translate(TG, glm::vec3(posFantasma[0], 0, posFantasma[1]));
    TG = glm::scale(TG, glm::vec3(0.65f / 0.25f)); // Alçada 0.65 / Orig 0.25
    TG = glm::translate(TG, -glm::vec3(0, 0, 0)); // Centro base Fantasma
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::modelTransformMoneda(float x, float z) {
    glm::mat4 TG(1.0f);
    TG = glm::translate(TG, glm::vec3(x, 0, z));
    TG = glm::scale(TG, glm::vec3(0.5f / 11.0f)); // Alçada 0.5 / Orig 11
    TG = glm::translate(TG, -glm::vec3(0, -5.5, -0.25)); // Centro base Moneda
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::modelTransformTorre() {
    glm::mat4 TG(1.0f);
    // Suponemos que la salida está en la fila 0 (x=0). Adyacente es x = -1.
    TG = glm::translate(TG, glm::vec3(posSortida[0] - 1, 0, posSortida[1]));

    // Rotar para encarar la salida. Si está en X=0, rotamos 90 grados en Y (dependerá de tu modelo, ajusta el ángulo si mira al revés)
    TG = glm::rotate(TG, (float)(M_PI / 2.0), glm::vec3(0, 1, 0));

    TG = glm::scale(TG, glm::vec3(6.0f / 172.0f)); // Alçada 6 / Orig 172
    TG = glm::translate(TG, -glm::vec3(-2, 0, -2)); // Centro base Torre
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::creaBuffersCub() {
    glm::vec3 v[8] = {glm::vec3(0,0,0), glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(1,1,0), glm::vec3(0,0,1), glm::vec3(1,0,1), glm::vec3(0,1,1), glm::vec3(1,1,1)};
    glm::vec3 p[36] = {v[0],v[2],v[1],v[1],v[2],v[3],v[5],v[1],v[7],v[1],v[3],v[7],v[2],v[6],v[3],v[3],v[6],v[7],v[0],v[4],v[6],v[0],v[6],v[2],v[0],v[1],v[4],v[1],v[5],v[4],v[4],v[5],v[6],v[5],v[7],v[6]};

    // Creamos un array con 36 colores grises (uno para cada vértice)
    glm::vec3 c[36];
    for(int i = 0; i < 36; ++i) {
        c[i] = glm::vec3(0.8f, 0.8f, 0.8f);
    }

    glGenVertexArrays(1, &VAO_Cub);
    glBindVertexArray(VAO_Cub);

    GLuint vbo[2];
    glGenBuffers(2, vbo);

    // 1. Vértices
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(p), p, GL_STATIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    // 2. Color (Matdiff)
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(c), c, GL_STATIC_DRAW);
    glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matdiffLoc);

    glBindVertexArray(0);
}

void MyGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h); ra = float(w) / float(h); projectTransform();
}

void MyGLWidget::keyPressEvent(QKeyEvent *event) {
    makeCurrent(); if (event->key() == Qt::Key_C) camaraGeneral = !camaraGeneral; update();
}

void MyGLWidget::inicialitzaMonedes() {
    int c = 0;
    for (int i = 0; i < N && c < 10; ++i) {
        for (int j = 0; j < M && c < 10; ++j) {
            if (laberint[i][j] == 0) { // Si hay suelo libre
                posMonedes[c].x = i;
                posMonedes[c].z = j;
                c++;
            }
        }
    }
}
