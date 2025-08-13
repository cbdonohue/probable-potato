#!/bin/bash

# SwarmApp Deployment Script
# Usage: ./deploy.sh [environment] [stack-name]

set -e

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Default values
ENVIRONMENT=${1:-production}
STACK_NAME=${2:-swarm-app}
COMPOSE_FILE="$PROJECT_ROOT/docker/docker-compose.swarm.yml"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if Docker Swarm is initialized
check_swarm() {
    if ! docker info | grep -q "Swarm: active"; then
        log_error "Docker Swarm is not initialized"
        log_info "Run 'docker swarm init' to initialize Swarm"
        exit 1
    fi
}

# Check if required files exist
check_files() {
    if [ ! -f "$COMPOSE_FILE" ]; then
        log_error "Compose file not found: $COMPOSE_FILE"
        exit 1
    fi
}

# Deploy the stack
deploy_stack() {
    log_info "Deploying $STACK_NAME to $ENVIRONMENT environment..."
    
    # Create overlay network if it doesn't exist
    if ! docker network ls | grep -q "swarm-network"; then
        log_info "Creating overlay network..."
        docker network create --driver overlay swarm-network
    fi
    
    # Deploy the stack
    docker stack deploy -c "$COMPOSE_FILE" "$STACK_NAME"
    
    log_success "Stack $STACK_NAME deployed successfully"
}

# Check deployment status
check_status() {
    log_info "Checking deployment status..."
    
    # Wait for services to be ready
    sleep 10
    
    # Show stack services
    docker stack services "$STACK_NAME"
    
    # Show stack tasks
    docker stack ps "$STACK_NAME"
    
    # Check if all services are running
    local failed_services=$(docker stack services "$STACK_NAME" --format "table {{.Name}}\t{{.Replicas}}" | grep -v "NAME" | grep -v "1/1\|3/3")
    
    if [ -n "$failed_services" ]; then
        log_warning "Some services may not be fully deployed:"
        echo "$failed_services"
    else
        log_success "All services are running successfully"
    fi
}

# Rollback function
rollback() {
    log_warning "Rolling back deployment..."
    
    # Remove the current stack
    docker stack rm "$STACK_NAME"
    
    # Wait for removal to complete
    while docker stack ls | grep -q "$STACK_NAME"; do
        sleep 2
    done
    
    log_info "Rollback completed"
}

# Health check
health_check() {
    log_info "Performing health check..."
    
    # Wait for services to be ready
    sleep 15
    
               # Get the service port
           local port=$(docker service ls --format "table {{.Name}}\t{{.Ports}}" | grep "$STACK_NAME" | awk '{print $2}' | cut -d':' -f2 | cut -d'-' -f1)
    
    if [ -n "$port" ]; then
        log_info "Testing endpoint on port $port..."
        
        # Test the health endpoint
        if curl -f -s "http://localhost:$port/health" > /dev/null; then
            log_success "Health check passed"
        else
            log_error "Health check failed"
            return 1
        fi
    else
        log_warning "Could not determine service port for health check"
    fi
}

# Main execution
main() {
    log_info "Starting deployment process..."
    log_info "Environment: $ENVIRONMENT"
    log_info "Stack name: $STACK_NAME"
    log_info "Compose file: $COMPOSE_FILE"
    
    # Pre-deployment checks
    check_swarm
    check_files
    
    # Deploy
    deploy_stack
    
    # Check status
    check_status
    
    # Health check
    if health_check; then
        log_success "Deployment completed successfully!"
    else
        log_error "Deployment health check failed"
        rollback
        exit 1
    fi
}

# Handle script arguments
case "${1:-}" in
    "rollback")
        rollback
        ;;
    "status")
        check_status
        ;;
    "health")
        health_check
        ;;
    "help"|"-h"|"--help")
        echo "Usage: $0 [environment] [stack-name]"
        echo "Commands:"
        echo "  $0 [environment] [stack-name]  - Deploy the application"
        echo "  $0 rollback                    - Rollback the deployment"
        echo "  $0 status                      - Check deployment status"
        echo "  $0 health                      - Perform health check"
        echo "  $0 help                        - Show this help"
        echo ""
        echo "Examples:"
        echo "  $0 production swarm-app"
        echo "  $0 staging swarm-app-staging"
        ;;
    *)
        main
        ;;
esac
