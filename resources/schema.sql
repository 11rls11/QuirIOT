-- ============================================
-- QuirIOT - Base de Datos
-- ============================================

DROP DATABASE IF EXISTS quiriot_db;
CREATE DATABASE quiriot_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE quiriot_db;

-- ============================================
-- USUARIOS Y ROLES
-- ============================================

CREATE TABLE usuarios (
    id_usuario INT AUTO_INCREMENT PRIMARY KEY,
    email VARCHAR(50) NOT NULL UNIQUE,
    contrasena VARCHAR(255) NOT NULL COMMENT 'Hash BCrypt',
    nombre VARCHAR(50) NOT NULL,
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_email (email)
) ENGINE=InnoDB COMMENT='Usuarios del sistema';

CREATE TABLE rol_usuario (
    id INT AUTO_INCREMENT PRIMARY KEY,
    rol ENUM('ENFERMERO','MEDICO','ANESTESIOLOGO','ADMIN') NOT NULL,
    id_usuario INT NOT NULL,
    FOREIGN KEY (id_usuario) REFERENCES usuarios(id_usuario) ON DELETE CASCADE,
    INDEX idx_usuario_rol (id_usuario)
) ENGINE=InnoDB COMMENT='Roles asignados a usuarios';

-- ============================================
-- QUIROFANOS
-- ============================================

CREATE TABLE quirofanos (
    id_quirofano INT AUTO_INCREMENT PRIMARY KEY,
    nombre VARCHAR(50) DEFAULT 'Quirofano',
    capacidad INT DEFAULT 10,
    disponibilidad VARCHAR(255) DEFAULT '24/7', -- Simplificado del SET para compatibilidad
    estado ENUM('DISPONIBLE','OCUPADO','MANTENIMIENTO','SANITIZANDO') DEFAULT 'DISPONIBLE',
    sistema_limpieza_activo BOOLEAN DEFAULT FALSE COMMENT 'Estado del sistema de limpieza automatico',
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_estado (estado)
) ENGINE=InnoDB COMMENT='Quirofanos del hospital';

-- ============================================
-- RESERVAS
-- ============================================

CREATE TABLE reservan (
    id_reserva INT AUTO_INCREMENT PRIMARY KEY,
    id_usuario INT NOT NULL,
    id_quirofano INT NOT NULL,
    fecha_inicio DATETIME NOT NULL,
    fecha_fin DATETIME NOT NULL,
    motivo_cirugia VARCHAR(255),
    estado_reserva ENUM('PROGRAMADA','EN_CURSO','COMPLETADA','CANCELADA') DEFAULT 'PROGRAMADA',
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (id_usuario) REFERENCES usuarios(id_usuario) ON DELETE CASCADE,
    FOREIGN KEY (id_quirofano) REFERENCES quirofanos(id_quirofano) ON DELETE CASCADE,
    INDEX idx_fecha_inicio (fecha_inicio),
    UNIQUE KEY unique_reserva_quirofano (id_quirofano, fecha_inicio, fecha_fin)
) ENGINE=InnoDB COMMENT='Reservas de cirugias';

-- ============================================
-- SENSORES Y MEDICIONES
-- ============================================

CREATE TABLE sensores (
    id_sensor INT AUTO_INCREMENT PRIMARY KEY,
    id_quirofano INT NOT NULL,
    tipo_sensor ENUM('TEMPERATURA','HUMEDAD','CALIDAD_AIRE') NOT NULL,
    modelo VARCHAR(50) DEFAULT 'Generico',
    estado ENUM('ACTIVO','INACTIVO','FALLA') DEFAULT 'ACTIVO',
    FOREIGN KEY (id_quirofano) REFERENCES quirofanos(id_quirofano) ON DELETE CASCADE,
    UNIQUE KEY unique_sensor_quirofano (id_quirofano, tipo_sensor)
) ENGINE=InnoDB COMMENT='Sensores fisicos instalados';

-- Esta tabla es la que tu código C++ usará para guardar el historial
CREATE TABLE mediciones (
    id_medicion INT AUTO_INCREMENT PRIMARY KEY,
    id_sensor INT NOT NULL,
    valor DECIMAL(10, 2) NOT NULL,
    fecha_hora TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (id_sensor) REFERENCES sensores(id_sensor) ON DELETE CASCADE,
    INDEX idx_sensor_fecha (id_sensor, fecha_hora)
) ENGINE=InnoDB COMMENT='Historial de lecturas de sensores';

-- ============================================
-- ACTUADORES E HISTORIAL
-- ============================================

CREATE TABLE actuadores (
    id_actuador INT AUTO_INCREMENT PRIMARY KEY,
    id_quirofano INT NOT NULL,
    tipo_actuador ENUM('VENTILADOR','LED_LIMPIEZA','FILTRO_AIRE') NOT NULL,
    estado_actual BOOLEAN DEFAULT FALSE,
    FOREIGN KEY (id_quirofano) REFERENCES quirofanos(id_quirofano) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='Actuadores controlables';

-- Mantenemos tu tabla personalizada de historial de limpieza
CREATE TABLE historial_sistema_limpieza (
    id_historial INT AUTO_INCREMENT PRIMARY KEY,
    id_quirofano INT NOT NULL,
    id_usuario INT NOT NULL,
    accion ENUM('ACTIVAR','DESACTIVAR') NOT NULL,
    fecha_accion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    razon VARCHAR(255),
    FOREIGN KEY (id_usuario) REFERENCES usuarios(id_usuario) ON DELETE CASCADE,
    FOREIGN KEY (id_quirofano) REFERENCES quirofanos(id_quirofano) ON DELETE CASCADE,
    INDEX idx_quirofano_fecha (id_quirofano, fecha_accion)
) ENGINE=InnoDB COMMENT='Bitacora de activacion manual del sistema';

-- ============================================
-- DATOS EJEMPLO
-- ============================================

-- Usuarios
INSERT INTO usuarios (email, contrasena, nombre) VALUES
('admin@quiriot.com', '$2a$10$Y50UaMFOxteibQEYLrwuAOoUh2sUd4CFjf2JTp5HUNk8SU9dcIa52', 'Dr. Admin'),
('medico1@quiriot.com', '$2a$10$Y50UaMFOxteibQEYLrwuAOoUh2sUd4CFjf2JTp5HUNk8SU9dcIa52', 'Dr. Garcia');

INSERT INTO rol_usuario (rol, id_usuario) VALUES ('ADMIN', 1), ('MEDICO', 2);

-- Quirofanos
INSERT INTO quirofanos (nombre, capacidad) VALUES
('Quirofano 1 - General', 10);

-- Sensores
-- Registramos los 3 tipos de sensores para el Quirofano 1
INSERT INTO sensores (id_quirofano, tipo_sensor, modelo) VALUES
(1, 'TEMPERATURA', 'DHT11'),
(1, 'HUMEDAD', 'DHT11'),
(1, 'CALIDAD_AIRE', 'MQ135');

-- Actuadores
INSERT INTO actuadores (id_quirofano, tipo_actuador) VALUES
(1, 'VENTILADOR'),
(1, 'LED_LIMPIEZA');

-- Reservas Ejemplo
INSERT INTO reservan (id_usuario, id_quirofano, fecha_inicio, fecha_fin, motivo_cirugia) VALUES
(2, 1, DATE_ADD(NOW(), INTERVAL 1 DAY), DATE_ADD(NOW(), INTERVAL '1 2' DAY_HOUR), 'Apendicectomia');

-- ============================================
-- CONSULTAS DE VERIFICACIÓN
-- ============================================
SELECT * FROM sensores;
SELECT * FROM mediciones;
