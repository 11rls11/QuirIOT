-- ============================================
-- QuirIOT - Base de Datos
-- ============================================

DROP DATABASE IF EXISTS quiriot_db;
CREATE DATABASE quiriot_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE quiriot_db;

-- Tabla de usuarios
CREATE TABLE usuarios (
    id_usuario INT(2) PRIMARY KEY AUTO_INCREMENT,
    email VARCHAR(50) NOT NULL UNIQUE,
    contrasena VARCHAR(255) NOT NULL COMMENT 'Hash de la contrasena',
    nombre VARCHAR(50) NOT NULL,
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_email (email)
) ENGINE=InnoDB COMMENT='Usuarios del sistema';

-- Tabla de roles
CREATE TABLE rol_usuario (
    id INT AUTO_INCREMENT PRIMARY KEY,
    rol ENUM('ENFERMERO','MEDICO','ANESTESIOLOGO') NOT NULL,
    id_usuario INT(2) NOT NULL,
    FOREIGN KEY (id_usuario) REFERENCES usuarios(id_usuario) ON DELETE CASCADE,
    INDEX idx_usuario_rol (id_usuario)
) ENGINE=InnoDB COMMENT='Roles de usuarios';

-- Tabla de quirofanos
CREATE TABLE quirofanos (
    id_quirofano INT(2) PRIMARY KEY AUTO_INCREMENT,
    disponibilidad SET('HORA_INICIO','HORA_FIN','FECHA') NOT NULL,
    nombre VARCHAR(50) DEFAULT 'Quirofano',
    capacidad INT DEFAULT 10,
    estado ENUM('DISPONIBLE','OCUPADO','MANTENIMIENTO','SANITIZANDO') DEFAULT 'DISPONIBLE',
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_estado (estado)
) ENGINE=InnoDB COMMENT='Quirofanos del hospital';

-- Tabla de reservas
CREATE TABLE reservan (
    id_reserva INT AUTO_INCREMENT PRIMARY KEY,
    id_usuario INT(2) NOT NULL,
    id_quirofano INT(2) NOT NULL,
    fecha_inicio DATETIME NOT NULL,
    fecha_fin DATETIME NOT NULL,
    motivo_cirugia VARCHAR(255),
    estado_reserva ENUM('PROGRAMADA','EN_CURSO','COMPLETADA','CANCELADA') DEFAULT 'PROGRAMADA',
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (id_usuario) REFERENCES usuarios(id_usuario) ON DELETE CASCADE,
    FOREIGN KEY (id_quirofano) REFERENCES quirofanos(id_quirofano) ON DELETE CASCADE,
    INDEX idx_fecha_inicio (fecha_inicio),
    INDEX idx_quirofano_fecha (id_quirofano, fecha_inicio),
    UNIQUE KEY unique_reserva_quirofano (id_quirofano, fecha_inicio, fecha_fin)
) ENGINE=InnoDB COMMENT='Reservas de cirugias';

-- Insertar usuarios de prueba
-- Contrasena: 123456 (hash BCrypt)
INSERT INTO usuarios (email, contrasena, nombre) VALUES
('admin@quiriot.com', '$2a$10$Y50UaMFOxteibQEYLrwuAOoUh2sUd4CFjf2JTp5HUNk8SU9dcIa52', 'Dr. Admin'),
('medico1@quiriot.com', '$2a$10$Y50UaMFOxteibQEYLrwuAOoUh2sUd4CFjf2JTp5HUNk8SU9dcIa52', 'Dr. Garcia'),
('medico2@quiriot.com', '$2a$10$Y50UaMFOxteibQEYLrwuAOoUh2sUd4CFjf2JTp5HUNk8SU9dcIa52', 'Dra. Martinez'),
('enfermero1@quiriot.com', '$2a$10$Y50UaMFOxteibQEYLrwuAOoUh2sUd4CFjf2JTp5HUNk8SU9dcIa52', 'Enf. Lopez');

-- Asignar roles
INSERT INTO rol_usuario (rol, id_usuario) VALUES
('MEDICO', 1),
('MEDICO', 2),
('MEDICO', 3),
('ENFERMERO', 4);

-- Insertar quirofanos de prueba
INSERT INTO quirofanos (disponibilidad, nombre, estado, capacidad) VALUES
('HORA_INICIO,HORA_FIN,FECHA', 'Quirofano 1 - Cirugia General', 'DISPONIBLE', 10),
('HORA_INICIO,HORA_FIN,FECHA', 'Quirofano 2 - Traumatologia', 'DISPONIBLE', 8),
('HORA_INICIO,HORA_FIN,FECHA', 'Quirofano 3 - Cardiologia', 'DISPONIBLE', 12);

-- Insertar una reserva de ejemplo
INSERT INTO reservan (id_usuario, id_quirofano, fecha_inicio, fecha_fin, motivo_cirugia, estado_reserva) VALUES
(2, 1, '2025-10-22 08:00:00', '2025-10-22 10:00:00', 'Apendicectomia', 'PROGRAMADA'),
(2, 1, '2025-10-22 11:00:00', '2025-10-22 13:00:00', 'Colecistectomia', 'PROGRAMADA'),
(3, 2, '2025-10-22 09:00:00', '2025-10-22 11:30:00', 'Fractura de femur', 'PROGRAMADA');

SELECT 'Base de datos creada exitosamente' AS mensaje;
SELECT COUNT(*) AS total_usuarios FROM usuarios;
SELECT COUNT(*) AS total_quirofanos FROM quirofanos;
SELECT COUNT(*) AS total_reservas FROM reservan;

-- Mostrar usuarios creados
SELECT id_usuario, email, nombre FROM usuarios;

-- Mostrar quirofanos
SELECT id_quirofano, nombre, estado FROM quirofanos;

-- Mostrar reservas del dia
SELECT
    r.id_reserva,
    u.nombre AS medico,
    q.nombre AS quirofano,
    r.fecha_inicio,
    r.fecha_fin,
    r.motivo_cirugia,
    r.estado_reserva
FROM reservan r
INNER JOIN usuarios u ON r.id_usuario = u.id_usuario
INNER JOIN quirofanos q ON r.id_quirofano = q.id_quirofano
ORDER BY r.fecha_inicio;

-- Tabla para registrar activaciones/desactivaciones del sistema
CREATE TABLE historial_sistema_limpieza (
    id_historial INT AUTO_INCREMENT PRIMARY KEY,
    id_quirofano INT(2) NOT NULL,
    id_usuario INT(2) NOT NULL,
    accion ENUM('ACTIVAR','DESACTIVAR') NOT NULL,
    fecha_accion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    razon VARCHAR(255),
    FOREIGN KEY (id_usuario) REFERENCES usuarios(id_usuario) ON DELETE CASCADE,
    FOREIGN KEY (id_quirofano) REFERENCES quirofanos(id_quirofano) ON DELETE CASCADE,
    INDEX idx_quirofano_fecha (id_quirofano, fecha_accion)
) ENGINE=InnoDB COMMENT='Historial de activacion/desactivacion del sistema de limpieza';

-- Agregar columna de estado del sistema de limpieza a quirofanos
ALTER TABLE quirofanos
ADD COLUMN sistema_limpieza_activo BOOLEAN DEFAULT TRUE
COMMENT 'Indica si el sistema de limpieza automatico esta activo';

-- Insertar datos de ejemplo
INSERT INTO historial_sistema_limpieza (id_quirofano, id_usuario, accion, razon) VALUES
(1, 1, 'ACTIVAR', 'Inicio de operaciones'),
(2, 1, 'ACTIVAR', 'Inicio de operaciones'),
(3, 1, 'ACTIVAR', 'Inicio de operaciones');
