# NodeStock

**NodeStock** es una aplicación de escritorio multiplataforma, desarrollada en C++ utilizando Qt, diseñada para la gestión de inventario de productos.

## Tabla de Contenidos

1. [Características](#características)
2. [Requisitos del Sistema](#requisitos-del-sistema)
3. [Contribuir al Proyecto](#contribuir-al-proyecto)
4. [Nomenclatura de Commits](#nomenclatura-de-commits)
5. [Licencia](#licencia)

## Características

- Interfaz de usuario moderna con Qt
- CRUD completo
- Soporte para archivos BLOB (imágenes u otros archivos)
- Optimizado para Linux

## Requisitos del Sistema

- Linux (Ubuntu, Fedora, etc.)
- C++11 o superior
- Qt 5 o superior

## Contribuir al Proyecto

Sigue estos pasos para colaborar:

1. **Clona** el repositorio en tu máquina local:
   ```bash
   git clone https://github.com/tu-usuario/nombre-del-proyecto.git
   ```
2. Navega al directorio del proyecto:
   ```bash
   cd nombre-del-proyecto
   ```
3. Crea una **branch** dedicada a la funcionalidad o corrección que estás implementando:
   ```bash
   git checkout -b nombre-de-tu-branch
   ```
4. Realiza los cambios y haz **commits** siguiendo la [nomenclatura de commits](#nomenclatura-de-commits):
   ```bash
   git add .
   git commit -m "tipo(scope): descripción"
   ```
5. Haz **push** de tu branch al repositorio remoto:
   ```bash
   git push origin nombre-de-tu-branch
   ```

### Reglas para Colaborar

- Mantén el código bien estructurado y asegúrate de que siga las convenciones establecidas por el equipo.
- Asegúrate de probar localmente los cambios antes de hacer push.

## Nomenclatura de Commits

Utiliza el siguiente formato para los mensajes de commit, asegurándote de que sean claros y concisos:

```
tipo(scope): descripción
```

### Tipos de Commits

- **feat**: Añade una nueva funcionalidad (ej. `feat(UI): add user login form`).
- **fix**: Corrige un bug (ej. `fix(database): resolve null pointer exception`).
- **docs**: Cambios en la documentación (ej. `docs: update README with installation steps`).
- **style**: Cambios de formato o estilo de código, sin impacto funcional (ej. `style: format code`).
- **refactor**: Refactorización del código (ej. `refactor: optimize database query`).
- **test**: Añade o modifica tests (ej. `test: add unit tests for user login`).
- **chore**: Cambios en herramientas o configuración (ej. `chore: update build script`).

### Ejemplos de Commits

```bash
feat(auth): implement user authentication system
fix(ui): correct button alignment on login form
docs: update API documentation for new endpoints
```

## Licencia

Este proyecto está bajo la licencia [Licencia], consulta el archivo `LICENSE` para más detalles.
