#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "model.h" // Usamos la ruta donde lo tienes

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    MyGLWidget(QWidget *parent = nullptr);
    ~MyGLWidget();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    // --- VAOs Iteración 1 ---
    GLuint VAO_Cub;
    GLuint VAO_Morty, VAO_Fantasma, VAO_Moneda, VAO_Torre;

    void creaBuffersCub();
    void creaBuffersModel(Model &m, GLuint &vao);

    // --- Modelos ---
    Model morty, fantasma, moneda, torre;

    // --- Laberinto (5x15) ---
    static const int N = 5;
    static const int M = 15;
    int laberint[N][M];

    // --- Shaders y Uniforms ---
    QOpenGLShaderProgram *program;
    GLuint vertexLoc, normalLoc, matambLoc, matdiffLoc, matspecLoc, matshinLoc;
    GLuint transLoc, projLoc, viewLoc;

    bool camaraGeneral;
    float fov, ra, znear, zfar;

    void projectTransform();
    void viewTransform();
    void modelTransformSuelo(int x, int z);
    void modelTransformPared(int x, int z);

    int posMorty[2];
    int posFantasma[2];
    int posSortida[2];

    struct MonedaPos { float x, z; };
    MonedaPos posMonedes[10];
    void inicialitzaMonedes(); // Función para repartirlas
    void modelTransformMorty();
    void modelTransformFantasma();
    void modelTransformMoneda(float x, float z);
    void modelTransformTorre();
};
#endif
