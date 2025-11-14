# Web Server Implementation

---lp-meta
title: Simple HTTP Web Server
language: python
namespace: webserver
author: XInnoDB Team
version: 1.0.0
---

A complete HTTP server implementation demonstrating literate programming principles. This document serves as both executable code and comprehensive documentation.

## Table of Contents

1. [Architecture Overview](#architecture)
2. [Module Structure](#structure)
3. [Middleware System](#middleware)
4. [Routing System](#routing)
5. [Server Implementation](#server)
6. [Usage Examples](#usage)

---

## Architecture Overview {#architecture}

Our web server follows a layered architecture with three main components:

```
Request Flow:
Client → Middleware Chain → Router → Handler → Response
```

**Key Design Decisions:**

- **Middleware pattern**: Composable request/response processing
- **Declarative routing**: Simple path-to-handler mapping
- **Chainable API**: Fluent interface for server configuration
- **Type-safe**: Full type hints for better IDE support

---

## Module Structure {#structure}

The complete module is built from these components:

```python ⟨ * ⟩
#!/usr/bin/env python3
"""⟨ module docstring ⟩"""

⟨ imports ⟩
⟨ configuration ⟩
⟨ middleware system ⟩
⟨ routing system ⟩
⟨ server class ⟩
⟨ example handlers ⟩
⟨ main entry point ⟩
```

### Documentation String

```python ⟨ module docstring ⟩
Simple HTTP Server with Middleware and Routing

This module provides a lightweight HTTP server with support for:
- Middleware chains for request/response processing
- Path-based routing
- Chainable configuration API
- Full type hints

Example:
    server = Server()
    server.use(logging_middleware)
    server.route('/api/users', users_handler)
    server.run()
```

### Dependencies

Core Python standard library imports:

```python ⟨ imports ⟩
from http.server import HTTPServer, BaseHTTPRequestHandler
from typing import Callable, Dict, List, Optional, Any
import json
import logging
from urllib.parse import urlparse, parse_qs
```

Later we add datetime support for timestamps:

```python ⟨ imports ⟩+
from datetime import datetime
from functools import wraps
```

And exception handling utilities:

```python ⟨ imports ⟩+
import traceback
from contextlib import contextmanager
```

---

## Configuration {#configuration}

Centralized configuration constants:

```python ⟨ configuration ⟩
# Server settings
DEFAULT_HOST = '0.0.0.0'
DEFAULT_PORT = 8080
MAX_REQUEST_SIZE = 1024 * 1024  # 1MB

# Logging configuration
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(name)s: %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

logger = logging.getLogger(__name__)
```

---

## Middleware System {#middleware}

Middleware functions intercept requests and responses, enabling cross-cutting concerns like logging, authentication, and CORS.

### Type Definitions

A middleware is a function that receives request and response dictionaries and returns a boolean indicating whether processing should continue:

```python ⟨ middleware system ⟩
# Type aliases
Middleware = Callable[[Dict[str, Any], Dict[str, Any]], bool]

class MiddlewareChain:
    """Manages a chain of middleware functions."""
    
    def __init__(self):
        ⟨ middleware::init fields ⟩
    
    ⟨ middleware::methods ⟩
```

### Fields

```python ⟨ middleware::init fields ⟩
self.middlewares: List[Middleware] = []
```

### Methods

Registration method adds middleware to the chain:

```python ⟨ middleware::methods ⟩
def use(self, middleware: Middleware) -> None:
    """Register a middleware function.
    
    Args:
        middleware: Function that processes request/response
    """
    self.middlewares.append(middleware)
```

Execution method runs all middleware in sequence:

```python ⟨ middleware::methods ⟩+
def execute(self, request: Dict[str, Any], response: Dict[str, Any]) -> bool:
    """Execute all middlewares in registration order.
    
    Returns False to stop processing (short-circuit).
    
    Args:
        request: Request dictionary
        response: Response dictionary
        
    Returns:
        True to continue, False to stop
    """
    for middleware in self.middlewares:
        try:
            if not middleware(request, response):
                logger.info("Middleware stopped request processing")
                return False
        except Exception as e:
            logger.error(f"Middleware error: {e}")
            logger.debug(traceback.format_exc())
            response['status'] = 500
            response['body'] = {'error': 'Internal server error'}
            return False
    
    return True
```

### Example Middleware

Here are some common middleware implementations:

```python ⟨ example middleware ⟩
def logging_middleware(request: Dict[str, Any], response: Dict[str, Any]) -> bool:
    """Log all incoming requests."""
    logger.info(
        f"{request['method']} {request['path']} "
        f"from {request.get('client_address', 'unknown')}"
    )
    return True

def cors_middleware(request: Dict[str, Any], response: Dict[str, Any]) -> bool:
    """Add CORS headers to response."""
    response['headers']['Access-Control-Allow-Origin'] = '*'
    response['headers']['Access-Control-Allow-Methods'] = 'GET, POST, PUT, DELETE, OPTIONS'
    response['headers']['Access-Control-Allow-Headers'] = 'Content-Type, Authorization'
    
    # Handle preflight requests
    if request['method'] == 'OPTIONS':
        response['status'] = 204
        return False  # Stop processing for OPTIONS
    
    return True

def json_middleware(request: Dict[str, Any], response: Dict[str, Any]) -> bool:
    """Parse JSON request body and set JSON response headers."""
    # Parse request body if JSON
    if request.get('headers', {}).get('content-type') == 'application/json':
        try:
            if request.get('body'):
                request['json'] = json.loads(request['body'])
        except json.JSONDecodeError as e:
            logger.warning(f"Invalid JSON in request: {e}")
            response['status'] = 400
            response['body'] = {'error': 'Invalid JSON'}
            return False
    
    # Set response content type
    response['headers']['Content-Type'] = 'application/json'
    return True

def timing_middleware(request: Dict[str, Any], response: Dict[str, Any]) -> bool:
    """Add request timing information."""
    request['start_time'] = datetime.now()
    return True
```

Add timing completion in response:

```python ⟨ example middleware ⟩+
def timing_completion_middleware(request: Dict[str, Any], response: Dict[str, Any]) -> bool:
    """Calculate and log request duration."""
    if 'start_time' in request:
        duration = (datetime.now() - request['start_time']).total_seconds()
        response['headers']['X-Request-Duration'] = str(duration)
        logger.debug(f"Request completed in {duration:.3f}s")
    return True
```

Authentication middleware example:

```python ⟨ example middleware ⟩+
def auth_middleware(request: Dict[str, Any], response: Dict[str, Any]) -> bool:
    """Simple token-based authentication."""
    # Skip auth for public endpoints
    if request['path'].startswith('/public'):
        return True
    
    auth_header = request.get('headers', {}).get('authorization', '')
    
    if not auth_header.startswith('Bearer '):
        response['status'] = 401
        response['body'] = {'error': 'Unauthorized: Missing or invalid token'}
        return False
    
    token = auth_header[7:]  # Remove 'Bearer ' prefix
    
    # In real implementation, validate token against database
    if token != 'secret-token':
        response['status'] = 403
        response['body'] = {'error': 'Forbidden: Invalid token'}
        return False
    
    request['user'] = {'id': 1, 'username': 'admin'}  # Add user to request
    return True
```

---

## Routing System {#routing}

The router maps URL paths to handler functions.

### Type Definitions

```python ⟨ routing system ⟩
RouteHandler = Callable[[Dict[str, Any]], Dict[str, Any]]

class Router:
    """URL routing with exact and pattern matching."""
    
    def __init__(self):
        ⟨ routing::init fields ⟩
    
    ⟨ routing::methods ⟩
```

### Fields

```python ⟨ routing::init fields ⟩
self.routes: Dict[str, RouteHandler] = {}
```

### Methods

Register a route handler:

```python ⟨ routing::methods ⟩
def route(self, path: str, handler: RouteHandler) -> None:
    """Register a route handler for exact path match.
    
    Args:
        path: URL path (e.g., '/api/users')
        handler: Function that processes the request
    """
    self.routes[path] = handler
    logger.debug(f"Registered route: {path}")
```

Find handler for a request path:

```python ⟨ routing::methods ⟩+
def match(self, path: str) -> Optional[RouteHandler]:
    """Find handler for the given path.
    
    Currently supports only exact matching.
    Future: Add pattern matching, path parameters.
    
    Args:
        path: Request path
        
    Returns:
        Handler function or None if not found
    """
    return self.routes.get(path)
```

Get all registered routes:

```python ⟨ routing::methods ⟩+
def list_routes(self) -> List[str]:
    """List all registered route paths.
    
    Returns:
        List of paths
    """
    return list(self.routes.keys())
```

---

## Server Implementation {#server}

The main server class orchestrates middleware and routing.

```python ⟨ server class ⟩
class Server:
    """HTTP server with middleware and routing support."""
    
    def __init__(self, host: str = DEFAULT_HOST, port: int = DEFAULT_PORT):
        ⟨ server::init body ⟩
    
    ⟨ server::public methods ⟩
    
    ⟨ server::private methods ⟩
```

### Initialization

```python ⟨ server::init body ⟩
self.host = host
self.port = port
self.router = Router()
self.middleware = MiddlewareChain()
logger.info(f"Server initialized on {host}:{port}")
```

### Public Methods

Chainable API for adding middleware:

```python ⟨ server::public methods ⟩
def use(self, middleware: Middleware) -> 'Server':
    """Add middleware to the chain (chainable).
    
    Args:
        middleware: Middleware function
        
    Returns:
        self for chaining
    """
    self.middleware.use(middleware)
    return self
```

Chainable API for registering routes:

```python ⟨ server::public methods ⟩+
def route(self, path: str, handler: RouteHandler) -> 'Server':
    """Register a route handler (chainable).
    
    Args:
        path: URL path
        handler: Handler function
        
    Returns:
        self for chaining
    """
    self.router.route(path, handler)
    return self
```

Start the server:

```python ⟨ server::public methods ⟩+
def run(self) -> None:
    """Start the HTTP server (blocking)."""
    logger.info(f"Starting server on http://{self.host}:{self.port}")
    logger.info(f"Registered routes: {', '.join(self.router.list_routes())}")
    
    try:
        ⟨ server::run implementation ⟩
    except KeyboardInterrupt:
        logger.info("Server stopped by user")
    except Exception as e:
        logger.error(f"Server error: {e}")
        logger.debug(traceback.format_exc())
```

### Run Implementation

Create and start the HTTP server:

```python ⟨ server::run implementation ⟩
handler_class = self._create_handler_class()
httpd = HTTPServer((self.host, self.port), handler_class)
httpd.serve_forever()
```

### Private Methods

Create a custom request handler class:

```python ⟨ server::private methods ⟩
def _create_handler_class(self) -> type:
    """Create a BaseHTTPRequestHandler subclass bound to this server.
    
    Returns:
        Handler class
    """
    server = self
    
    class RequestHandler(BaseHTTPRequestHandler):
        def do_GET(self):
            ⟨ server::handle request ⟩
        
        def do_POST(self):
            ⟨ server::handle request ⟩
        
        def do_PUT(self):
            ⟨ server::handle request ⟩
        
        def do_DELETE(self):
            ⟨ server::handle request ⟩
        
        def do_OPTIONS(self):
            ⟨ server::handle request ⟩
        
        def log_message(self, format, *args):
            # Suppress default logging (we have our own)
            pass
    
    return RequestHandler
```

Handle incoming requests:

```python ⟨ server::handle request ⟩
# Build request dictionary
request = {
    'method': self.command,
    'path': self.path,
    'headers': dict(self.headers),
    'client_address': self.client_address[0],
}

# Read request body if present
content_length = int(self.headers.get('content-length', 0))
if content_length > 0:
    if content_length > MAX_REQUEST_SIZE:
        self.send_error(413, "Request entity too large")
        return
    request['body'] = self.rfile.read(content_length).decode('utf-8')

# Parse query string
parsed = urlparse(self.path)
request['query'] = parse_qs(parsed.query)

# Initialize response
response = {
    'status': 200,
    'headers': {},
    'body': {}
}

# Execute middleware chain
if not server.middleware.execute(request, response):
    self._send_response(response)
    return

# Route to handler
handler = server.router.match(parsed.path)
if handler is None:
    response['status'] = 404
    response['body'] = {'error': f"Not found: {parsed.path}"}
else:
    try:
        result = handler(request)
        if result is not None:
            response['body'] = result
    except Exception as e:
        logger.error(f"Handler error: {e}")
        logger.debug(traceback.format_exc())
        response['status'] = 500
        response['body'] = {'error': 'Internal server error'}

# Send response
self._send_response(response)
```

Send HTTP response:

```python ⟨ server::handle request ⟩+

def _send_response(self, response: Dict[str, Any]) -> None:
    """Send HTTP response to client."""
    self.send_response(response['status'])
    
    # Send headers
    for key, value in response.get('headers', {}).items():
        self.send_header(key, value)
    
    # Prepare body
    body = response.get('body', {})
    if isinstance(body, (dict, list)):
        body_bytes = json.dumps(body).encode('utf-8')
        self.send_header('Content-Type', 'application/json')
    else:
        body_bytes = str(body).encode('utf-8')
    
    self.send_header('Content-Length', len(body_bytes))
    self.end_headers()
    
    # Send body
    self.wfile.write(body_bytes)
```

---

## Example Handlers {#handlers}

Sample route handlers demonstrating common patterns:

```python ⟨ example handlers ⟩
def health_handler(request: Dict[str, Any]) -> Dict[str, Any]:
    """Health check endpoint."""
    return {
        'status': 'healthy',
        'timestamp': datetime.now().isoformat(),
        'uptime': 'unknown'  # In real impl, track server start time
    }

def echo_handler(request: Dict[str, Any]) -> Dict[str, Any]:
    """Echo request data back."""
    return {
        'method': request['method'],
        'path': request['path'],
        'headers': request.get('headers', {}),
        'body': request.get('json', request.get('body')),
        'query': request.get('query', {})
    }

def time_handler(request: Dict[str, Any]) -> Dict[str, Any]:
    """Return current server time."""
    now = datetime.now()
    return {
        'timestamp': now.isoformat(),
        'unix': now.timestamp(),
        'formatted': now.strftime('%Y-%m-%d %H:%M:%S %Z')
    }

def user_info_handler(request: Dict[str, Any]) -> Dict[str, Any]:
    """Return authenticated user info (requires auth middleware)."""
    user = request.get('user')
    if user:
        return {'user': user}
    else:
        return {'error': 'No user information available'}
```

---

## Usage Examples {#usage}

### Basic Server

Minimal server with no middleware:

```python ⟨ usage example::basic ⟩
def run_basic_server():
    """Run a basic server with one route."""
    server = Server('localhost', 8000)
    server.route('/health', health_handler)
    server.run()
```

### Full-Featured Server

Server with all middleware and multiple routes:

```python ⟨ usage example::full ⟩
def run_full_server():
    """Run a fully-featured server."""
    server = Server()
    
    # Add middleware (order matters!)
    server.use(timing_middleware)
    server.use(logging_middleware)
    server.use(cors_middleware)
    server.use(json_middleware)
    # server.use(auth_middleware)  # Uncomment to enable auth
    server.use(timing_completion_middleware)
    
    # Register routes
    server.route('/health', health_handler)
    server.route('/echo', echo_handler)
    server.route('/time', time_handler)
    server.route('/user', user_info_handler)
    
    # Public routes (no auth required)
    server.route('/public/info', lambda req: {
        'name': 'Example Server',
        'version': '1.0.0'
    })
    
    # Start server
    server.run()
```

---

## Main Entry Point {#main}

Command-line entry point:

```python ⟨ main entry point ⟩
def main():
    """Main entry point."""
    import sys
    
    ⟨ example middleware ⟩
    ⟨ usage example::basic ⟩
    ⟨ usage example::full ⟩
    
    # Parse command line arguments
    if len(sys.argv) > 1 and sys.argv[1] == '--basic':
        run_basic_server()
    else:
        run_full_server()

if __name__ == '__main__':
    main()
```

---

## Testing

To test the server, run:

```bash
# Start server
python webserver.py

# In another terminal:
# Health check
curl http://localhost:8080/health

# Echo endpoint
curl -X POST http://localhost:8080/echo \
  -H "Content-Type: application/json" \
  -d '{"message": "Hello, World!"}'

# Time endpoint
curl http://localhost:8080/time

# Public info (no auth)
curl http://localhost:8080/public/info

# With authentication (will fail without token)
curl http://localhost:8080/user

# With authentication (will succeed)
curl http://localhost:8080/user \
  -H "Authorization: Bearer secret-token"
```

---

## Summary

This literate program demonstrates:

✅ **Named chunks**: Code organized into logical, reusable pieces  
✅ **Additive composition**: Extending chunks with `+` operator  
✅ **Cross-references**: Chunks referencing other chunks  
✅ **Documentation**: Code explanation in natural prose  
✅ **Executable**: Tangle to produce working Python code  
✅ **Readable**: Valid Markdown rendering on GitHub  

**To extract executable code:**
```bash
literate tangle webserver.lit.md --chunk "*" -o webserver.py
python webserver.py
```

**To generate documentation:**
```bash
literate weave webserver.lit.md -o webserver.pdf
```

---

## License

This example is released into the public domain. Use it freely for learning and inspiration.

---

**End of Literate Program**


