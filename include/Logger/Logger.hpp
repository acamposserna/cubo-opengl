/**
 * @file Logger.hpp
 * 
 * @brief Utilidad para escribir mensajes de log tanto en consola como en fichero
 * 
 * @details
 * Proporciona una interfaz unificada para los mensajes de log, puede escribir tanto
 * en la consola (<code>cerr</code>) como en un fichero.
 * 
 * El formato de escritura del log es: "[LEVEL] [DD/MM/AAAA HH:MM:SS] mensaje".
 * 
 * La clase almacena un puntero a <code>std::ostream</code> que en el momento de
 * creación de la clase puede apuntar a un <code>std::ostream</code> puro (consola)
 * o a un <code>std::ofstream</code> (fichero).
 * 
 * El stream se almacena en un <code>std::unique_ptr</code> que cuando el objeto
 * <code>Logger</code> se destruye, el destructor del <code>unique_ptr</code>
 * invoca delete sobre el stream, cerrando el fichero si fuera necesario (patrón RAII).
 * 
 * <code>std::unique_ptr</code> no es copiable pero si movible, por tanto <code>Logger</code>
 * no pude copiarse pero si moverse, la propiedad del stream se transfiere al nuevo
 * objeto y el original queda en estado vacío.
 * 
 * @author Antonio Campos (a.campos.serna@gmail.com)
 * @version 0.1
 * @date 2026-05-21
 * @copyright GNU General Public License v3 (see https://www.gnu.org/licenses/gpl-3.0.html)
 */
#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include <memory>
#include <ostream>
#include <string>
#include <string_view>

/**
 * @brief Destinos de escritura del <code>Logger</code>: Consola o Fichero
 */
enum class LogMode {
    CONSOLE,
    FILE
};

/**
 * @brief Niveles de severidad del mensaje de log, están ordenados de menor a mayor
 */
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL    
};

/**
 * @brief Clase para escribir mensajes de log tanto en consola como en fichero.
 */
class Logger {
    private:
        // Nivel de severidad mínimo para escribir en el <code>Logger</code>
        LogLevel minLogLevel_;

        // Puntero a <code>std::ostream</code> donde escribe el <code>Logger</code>
        std::unique_ptr<std::ostream> stream_;

        /**
         * @brief Crea el objeto <code>std::ostream</code>
         * 
         * @param mode Destino de escritura (consola o fichero)
         * @param filename Nombre del fichero
         * @return std::unique_ptr<std::ostream> Puntero al stream del <code>Logger</code>
         */
        static std::unique_ptr<std::ostream> create_stream(
            LogMode mode,
            std::string_view filename);

        /**
         * @brief Convierte el nivel de severidad a una cadeana de texto
         * 
         * @param level Nivel de severidad
         * @return std::string Cadena de texto con la severidad
         */
        static std::string logLevelToString(LogLevel level);

        /**
         * @brief Obtiene la fecha y hora actual en el formato "DD/MM/AAAA HH:MM:SS"
         * 
         * @return std::string Cadena de texto con la fecha y hora
         */
        static std::string getCurrentTime();
    
    public:
        /**
         * @brief Constructor del objeto <code>Logger</code>
         * 
         * @param minLogLevel Nivel mínimo de severidad para escribir en el <code>Logger</code>
         * @param logMode Destino de escritura (consola o fichero)
         * @param filename Nombre del fichero
         */
        explicit Logger(LogLevel minLogLevel = LogLevel::DEBUG,
                        LogMode logMode = LogMode::CONSOLE,
                        std::string_view filename = "");

        /**
         * @brief Escribe un mensaje en el <code>Logger</code> si level >= Logger::minLogLevel_
         * 
         * @param level Nivel de severidad del mensaje
         * @param message Mensaje a escribir en el <code>Logger</code>
         */
        void log(LogLevel level, std::string_view message);

        std::ostream& stream() noexcept;

        /**
         * Sobrecarga de operadores
         */

        // Sobrecarga operador <<
        template<typename T> Logger& operator<< (const T& value) {
            *stream_ << value;
            return *this;
        }

        // Sobrecarga manipuladores (std::endl, std::flush…)
        Logger& operator<<(std::ostream& (*manip)(std::ostream&));

        /*
         * Regla de los cinco. Administración de recursos.
         */

        // La clase no es copiable pero si movible
        Logger(const Logger&)            = delete;   // No copiable
        Logger& operator=(const Logger&) = delete;   // No copiable

        Logger(Logger&&)            = default;       // Movible
        Logger& operator=(Logger&&) = default;       // Movible

        // El destructor de clase cierra automáticamente el fichero (RAII)
        ~Logger() = default;
};

#endif