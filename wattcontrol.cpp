
// =============================================================
// WattControl - Gestor de energia de un minimercado
// EA2: Herencia, polimorfismo y relaciones entre clases
// Adaptacion del modelo AquaControl del docente
// El Nino 2026
// Estudiate: Saira Dayana Ricardo Gil
// Ingenieria de Software y Datos
// =============================================================


// --- Librerias ---
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <ctime>


using namespace std;


// =============================================================
// DIRECTIVAS Y CONSTANTES DEL NEGOCIO
// =============================================================


#define HORAS_DIA 24
#define MAX_CARGAS 3


// Potencias de los equipos del minimercado
const float POTENCIA_REFRIGERADORES = 2.5;
const float POTENCIA_AIRE_ACONDICIONADO = 3.5;
const float POTENCIA_CALENTADOR = 2.0;


// Potencia maxima contratada para el negocio
const float POTENCIA_CONTRATADA = 7.0;


// Reglas del mercado energetico
const float PRECIO_CARO = 800.0;
const float PRECIO_EMERGENCIA = 1500.0;
const float NIVEL_ALERTA_EMBALSES = 40.0;


// Horas pico
const int PICO_INICIO = 18;
const int PICO_FIN = 20;


// Horas minimas de funcionamiento
// Valores iniciales que el equipo debe justificar en el informe.
const int HORAS_MIN_AIRE = 2;
const int HORAS_MIN_CALENTADOR = 1;


// Falla de lectura: probabilidad de 1 de cada 20 horas
const int FALLA_CADA = 20;


// =============================================================
// CLASE CARGA: CLASE BASE ABSTRACTA
// Equivale a Equipo en AquaControl
// =============================================================


class Carga {


protected:
    string nombre;
    float potenciaKW;
    bool encendida;
    int horasUso;


public:


    Carga(string n, float p) {
        nombre = n;
        potenciaKW = p;
        encendida = false;
        horasUso = 0;
    }


    virtual ~Carga() {}


    // Metodo virtual puro: permite el polimorfismo
    virtual bool debeEncender(int hora, float precio,
                              float embalses) = 0;


    // Permite identificar las cargas aplazables
    virtual bool esAplazable() {
        return false;
    }


    void encender() {
        encendida = true;
    }


    void apagar() {
        encendida = false;
    }


    // Registrar una hora de uso cuando el equipo esta encendido
    void registrarHoraUso() {
        if (encendida) {
            horasUso++;
        }
    }


    float costoHora(float precio) {
        if (encendida) {
            return potenciaKW * precio;
        }
        return 0;
    }


    float getPotencia() {
        return potenciaKW;
    }


    string getNombre() {
        return nombre;
    }


    bool estaEncendido() {
        return encendida;
    }


    int getHorasUso() {
        return horasUso;
    }


    float getKWh() {
        return potenciaKW * horasUso;
    }


    void reiniciarHoras() {
        horasUso = 0;
        encendida = false;
    }
};


// =============================================================
// HERENCIA: CARGA CRITICA
// Equivale a Bomba en AquaControl
// Regla 1: nunca se apaga
// =============================================================


class CargaCritica : public Carga {


public:


    CargaCritica(string n, float p) : Carga(n, p) {}


    bool debeEncender(int hora, float precio,
                      float embalses) override {


        return true;
    }
};


// =============================================================
// HERENCIA: CARGA APLAZABLE
// Equivale a Iluminacion en AquaControl
// Reglas 2 a 5
// =============================================================


class CargaAplazable : public Carga {


public:


    CargaAplazable(string n, float p) : Carga(n, p) {}


    bool esAplazable() override {
        return true;
    }


    bool debeEncender(int hora, float precio,
                      float embalses) override {


        bool horaPico =
            hora >= PICO_INICIO && hora <= PICO_FIN;


        // Reglas de aplazamiento:
        // 1. Embalses por debajo del 40 %.
        // 2. Hora pico.
        // 3. Precio superior a 800 COP/kWh.

        bool aplazar =
            (embalses < NIVEL_ALERTA_EMBALSES) ||
            horaPico ||
            (precio > PRECIO_CARO);


        return !aplazar;
    }
};


// =============================================================
// CLASE MEDIDOR
// Equivale al componente de medicion del sistema
// Relacion: COMPOSICION con Negocio
// =============================================================


class Medidor {


private:


    float energiaTotal;
    float costoTotal;


public:


    Medidor() {
        energiaTotal = 0;
        costoTotal = 0;
    }


    void registrar(float potencia, float precio) {


        // La potencia en kW durante una hora equivale a kWh
        float energia = potencia;


        energiaTotal += energia;
        costoTotal += energia * precio;
    }


    void reiniciar() {
        energiaTotal = 0;
        costoTotal = 0;
    }


    float getEnergia() {
        return energiaTotal;
    }


    float getCosto() {
        return costoTotal;
    }
};


// =============================================================
// CLASE MERCADO
// Equivale a Mercado en AquaControl
// Relacion: ASOCIACION con Negocio
// =============================================================


class Mercado {


public:


    // Genera el precio de acuerdo con la franja horaria
    float precioHora(int hora) {


        int minimo, maximo;


        if (hora <= 6) {
            minimo = 398;
            maximo = 640;
        }
        else if (hora <= 14) {
            minimo = 456;
            maximo = 970;
        }
        else if (hora <= 17) {
            minimo = 596;
            maximo = 1040;
        }
        else if (hora <= 20) {
            minimo = 705;
            maximo = 1612;
        }
        else {
            minimo = 565;
            maximo = 970;
        }


        return minimo + rand() % (maximo - minimo + 1);
    }


    // Nivel simulado de los embalses durante el dia
    float nivelEmbalses(int hora) {


        float nivel = 55.0 - (hora * 0.8);


        if (nivel < 25.0) {
            nivel = 25.0;
        }


        return nivel;
    }
};


// =============================================================
// CLASE NEGOCIO
// Equivale a Edificio en AquaControl
//
// COMPOSICION: contiene un Medidor.
// AGREGACION: recibe cargas creadas afuera.
// ASOCIACION: conoce al Mercado.
// =============================================================


class Negocio {


private:


    string nombre;


    // Composicion: el medidor pertenece al negocio
    Medidor medidor;


    // Agregacion: las cargas se crean fuera del negocio
    Carga* cargas[MAX_CARGAS];


    int numCargas;


    // Asociacion: el negocio conoce el mercado
    Mercado* mercado;


    // Resultados de la simulacion
    float costoCon;
    float costoSin;
    int horasSimuladas;


    // Para registrar si se produjo una emergencia
    bool emergencia;


    // Controlar que se haya realizado una simulacion
    bool simulacionRealizada;


    // Aplicar la regla de potencia contratada
    void controlarPotencia(float& potenciaActual) {


        if (potenciaActual > POTENCIA_CONTRATADA) {


            int posicionMayor = -1;
            float mayorPotencia = 0;


            for (int i = 0; i < numCargas; i++) {


                if (cargas[i]->estaEncendido() &&
                    cargas[i]->esAplazable() &&
                    cargas[i]->getPotencia() > mayorPotencia) {


                    mayorPotencia = cargas[i]->getPotencia();
                    posicionMayor = i;
                }
            }


            if (posicionMayor != -1) {


                cargas[posicionMayor]->apagar();


                potenciaActual -= mayorPotencia;


                cout << "\nALERTA: Se supera la potencia "
                     << "contratada.\n";


                cout << "Se apaga la carga aplazable de mayor "
                     << "potencia: "
                     << cargas[posicionMayor]->getNombre()
                     << endl;
            }
        }
    }


public:


    Negocio(string n, Mercado* m) {


        nombre = n;
        mercado = m;
        numCargas = 0;


        costoCon = 0;
        costoSin = 0;
        horasSimuladas = 0;
        emergencia = false;
        simulacionRealizada = false;
    }


    // Agregar cargas al negocio
    void agregarCarga(Carga* c) {


        if (numCargas < MAX_CARGAS) {


            cargas[numCargas] = c;
            numCargas++;
        }
    }


    // Mostrar informacion del minimercado
    void mostrar() {


        cout << "\n====================================\n";
        cout << "        WATTCONTROL - MINIMERCADO\n";
        cout << "====================================\n";


        cout << "Negocio: " << nombre << endl;


        cout << "Potencia contratada: "
             << POTENCIA_CONTRATADA << " kW\n";


        cout << "\nEquipos registrados:\n";


        for (int i = 0; i < numCargas; i++) {


            cout << " - " << cargas[i]->getNombre()
                 << " | Potencia: "
                 << cargas[i]->getPotencia()
                 << " kW\n";
        }
    }


    // Evaluar las decisiones de las cargas
    void evaluar(int hora, float precio, float embalses) {


        float potenciaActual = 0;


        cout << "\nLECTURA DEL NEGOCIO\n";


        cout << "Hora: " << hora << endl;
        cout << "Precio: $" << precio << " COP/kWh\n";
        cout << "Embalses: " << embalses << " %\n";


        cout << "\nDECISION DE LOS EQUIPOS\n";


        for (int i = 0; i < numCargas; i++) {


            bool encender =
                cargas[i]->debeEncender(hora, precio, embalses);


            if (encender) {


                cargas[i]->encender();


                potenciaActual += cargas[i]->getPotencia();


                cout << cargas[i]->getNombre()
                     << ": ENCENDER\n";
            }
            else {


                cargas[i]->apagar();


                cout << cargas[i]->getNombre()
                     << ": APLAZAR / APAGAR\n";
            }
        }


        // Regla 6: controlar la potencia contratada
        controlarPotencia(potenciaActual);


        cout << "Potencia final encendida: "
             << potenciaActual << " kW\n";
    }


    // Simular las 24 horas del dia
    void simularDia() {


        medidor.reiniciar();


        costoCon = 0;
        costoSin = 0;
        horasSimuladas = 0;
        emergencia = false;
        simulacionRealizada = true;


        // Reiniciar las horas de uso de los equipos
        for (int i = 0; i < numCargas; i++) {
            cargas[i]->reiniciarHoras();
        }


        cout << "\n============================================\n";
        cout << "         SIMULACION DE ENERGIA - 24 HORAS\n";
        cout << "============================================\n";


        cout << fixed << setprecision(2);


        cout << setw(5) << "Hora"
             << setw(10) << "Precio"
             << setw(12) << "Embalses";


        for (int i = 0; i < numCargas; i++) {
            cout << setw(14) << cargas[i]->getNombre();
        }


        cout << setw(12) << "Costo\n";


        // Ciclo de las 24 horas
        for (int h = 0; h < HORAS_DIA; h++) {


            // El negocio consulta al mercado
            float precio = mercado->precioHora(h);


            float embalses = mercado->nivelEmbalses(h);


            // Costo sin gestor: todas las cargas funcionan
            float potenciaTotal = 0;


            for (int i = 0; i < numCargas; i++) {
                potenciaTotal += cargas[i]->getPotencia();
            }


            costoSin += potenciaTotal * precio;


            // BREAK: emergencia de racionamiento
            if (precio > PRECIO_EMERGENCIA &&
                embalses < NIVEL_ALERTA_EMBALSES) {


                cout << "\nEMERGENCIA DE RACIONAMIENTO\n";
                cout << "Precio superior a $1500 y embalses "
                     << "por debajo del 40 %.\n";


                cout << "La simulacion se detiene.\n";


                emergencia = true;
                break;
            }


            horasSimuladas++;


            // CONTINUE: falla de lectura
            if (rand() % FALLA_CADA == 0) {


                cout << setw(5) << h
                     << setw(10) << precio
                     << setw(12) << embalses;


                float costoHora = 0;


                // Solo funciona la carga critica
                for (int i = 0; i < numCargas; i++) {


                    if (cargas[i]->esAplazable()) {


                        cargas[i]->apagar();


                        cout << setw(14) << "OFF";
                    }
                    else {


                        cargas[i]->encender();


                        cargas[i]->registrarHoraUso();


                        float potencia = cargas[i]->getPotencia();


                        costoHora +=
                            cargas[i]->costoHora(precio);


                        medidor.registrar(potencia, precio);


                        cout << setw(14) << "ON";
                    }
                }


                costoCon += costoHora;


                cout << setw(12) << costoHora
                     << "   Falla de lectura\n";


                continue;
            }


            float potenciaActual = 0;


            float costoHora = 0;


            cout << setw(5) << h
                 << setw(10) << precio
                 << setw(12) << embalses;


            // Cada carga decide usando el mismo metodo virtual
            for (int i = 0; i < numCargas; i++) {


                bool encender =
                    cargas[i]->debeEncender(h, precio, embalses);


                if (encender) {


                    cargas[i]->encender();


                    potenciaActual += cargas[i]->getPotencia();
                }
                else {


                    cargas[i]->apagar();
                }
            }


            // Regla 6: verificar potencia contratada
            controlarPotencia(potenciaActual);


            // Calcular consumo y costo de cada carga
            for (int i = 0; i < numCargas; i++) {


                if (cargas[i]->estaEncendido()) {


                    float potencia = cargas[i]->getPotencia();


                    costoHora += cargas[i]->costoHora(precio);


                    medidor.registrar(potencia, precio);


                    // Registrar una hora de uso por cada hora simulada
                    cargas[i]->registrarHoraUso();
                }


                cout << setw(14)
                     << (cargas[i]->estaEncendido() ? "ON" : "OFF");
            }


            costoCon += costoHora;


            cout << setw(12) << costoHora << endl;
        }


        cout << "\nSimulacion finalizada.\n";
        cout << "Consulte la opcion 4 para ver el resumen.\n";
    }


    // Mostrar el resumen y calcular el ahorro
    void resumen() {


        cout << fixed << setprecision(2);


        cout << "\n====================================\n";
        cout << "       RESUMEN WATTCONTROL\n";
        cout << "====================================\n";


        cout << "Negocio: " << nombre << endl;


        cout << "Horas simuladas: "
             << horasSimuladas << " de "
             << HORAS_DIA << endl;


        cout << "\nCONSUMO POR EQUIPO\n";


        for (int i = 0; i < numCargas; i++) {


            cout << "\nEquipo: " << cargas[i]->getNombre();


            cout << "\nHoras de uso: "
                 << cargas[i]->getHorasUso();


            cout << "\nEnergia consumida: "
                 << cargas[i]->getKWh() << " kWh\n";
        }


        cout << "\nEnergia total con gestor: "
             << medidor.getEnergia() << " kWh\n";


        cout << "Costo con gestor: $"
             << costoCon << " COP\n";


        cout << "Costo sin gestor: $"
             << costoSin << " COP\n";


        float ahorro = costoSin - costoCon;


        cout << "Ahorro obtenido: $"
             << ahorro << " COP\n";


        if (costoSin > 0) {


            float porcentaje = (ahorro / costoSin) * 100;


            cout << "Porcentaje de ahorro: "
                 << porcentaje << " %\n";
        }


        // Revisar las horas minimas de funcionamiento
        bool produccionCompleta = true;


        cout << "\nCONTROL DE HORAS MINIMAS\n";


        for (int i = 0; i < numCargas; i++) {


            int minimo = 0;


            if (cargas[i]->getNombre() == "Aire acondicionado") {
                minimo = HORAS_MIN_AIRE;
            }
            else if (cargas[i]->getNombre() == "Calentador de agua") {
                minimo = HORAS_MIN_CALENTADOR;
            }


            if (minimo > 0) {


                cout << cargas[i]->getNombre()
                     << ": " << cargas[i]->getHorasUso()
                     << " horas de " << minimo
                     << " horas minimas.\n";


                if (cargas[i]->getHorasUso() < minimo) {
                    produccionCompleta = false;
                }
            }
        }


        if (!produccionCompleta) {


            cout << "\nALERTA: PRODUCCION INCOMPLETA.\n";
            cout << "No se cumplieron las horas minimas "
                 << "de funcionamiento.\n";
        }
        else {


            cout << "\nProduccion completa: se cumplieron "
                 << "las horas minimas.\n";
        }


        if (emergencia) {
            cout << "\nLa simulacion termino por emergencia "
                 << "de racionamiento.\n";
        }
    }


    bool haySimulacion() {
        return simulacionRealizada;
    }
};


// =============================================================
// PROGRAMA PRINCIPAL
// Equivale al main de AquaControl
// =============================================================


int main() {


    // Inicializar numeros aleatorios
    srand(time(0));


    // El mercado existe independientemente del negocio
    Mercado mercado;


    // Crear las cargas fuera del negocio
    // Esto representa la AGREGACION
    CargaCritica refrigeradores(
        "Refrigeradores", POTENCIA_REFRIGERADORES
    );


    CargaAplazable aireAcondicionado(
        "Aire acondicionado", POTENCIA_AIRE_ACONDICIONADO
    );


    CargaAplazable calentador(
        "Calentador de agua", POTENCIA_CALENTADOR
    );


    // Crear el negocio y asociarlo con el mercado
    Negocio minimercado("Minimercado WattControl", &mercado);


    // Agregar las cargas al negocio
    minimercado.agregarCarga(&refrigeradores);
    minimercado.agregarCarga(&aireAcondicionado);
    minimercado.agregarCarga(&calentador);


    // Mostrar la informacion inicial
    minimercado.mostrar();


    // Variables para el menu
    int opcion;


    int hora = 12;


    float precio = 700;


    float embalses = 55;


    // Menu principal con do-while
    do {


        cout << "\n====================================\n";
        cout << "           MENU WATTCONTROL\n";
        cout << "====================================\n";


        cout << "1. Ingresar lectura manual\n";
        cout << "2. Evaluar decision de cada carga\n";
        cout << "3. Simular un dia completo (24 horas)\n";
        cout << "4. Ver resumen de la simulacion\n";
        cout << "0. Salir\n";


        cout << "Seleccione una opcion: ";
        cin >> opcion;


        switch (opcion) {


            case 1:


                // Validar hora
                do {


                    cout << "Ingrese la hora (0 a 23): ";
                    cin >> hora;


                    if (hora < 0 || hora >= HORAS_DIA) {
                        cout << "Hora invalida. Intente nuevamente.\n";
                    }


                } while (hora < 0 || hora >= HORAS_DIA);


                // Validar precio
                do {


                    cout << "Ingrese el precio de energia "
                         << "(COP/kWh): ";


                    cin >> precio;


                    if (precio <= 0) {
                        cout << "El precio debe ser mayor que cero.\n";
                    }


                } while (precio <= 0);


                // Validar nivel de embalses
                do {


                    cout << "Ingrese el nivel de los embalses "
                         << "(0 a 100 %): ";


                    cin >> embalses;


                    if (embalses < 0 || embalses > 100) {
                        cout << "Nivel invalido. Intente nuevamente.\n";
                    }


                } while (embalses < 0 || embalses > 100);


                cout << "\nLectura guardada correctamente.\n";


                break;


            case 2:


                minimercado.evaluar(hora, precio, embalses);


                break;


            case 3:


                minimercado.simularDia();


                break;


            case 4:


                if (minimercado.haySimulacion()) {


                    minimercado.resumen();


                }
                else {


                    cout << "\nPrimero debe realizar una simulacion.\n";
                }


                break;


            case 0:


                cout << "\nGracias por utilizar WattControl.\n";
                cout << "Hasta pronto.\n";


                break;


            default:


                cout << "\nOpcion invalida. Seleccione una opcion "
                     << "del menu.\n";
        }


    } while (opcion != 0);


    return 0;
}
