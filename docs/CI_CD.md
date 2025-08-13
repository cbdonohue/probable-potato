# GitLab CI/CD Setup

This document describes the CI/CD pipeline setup for the SwarmApp modular C++ application.

## Overview

The CI/CD pipeline automates the build, test, and deployment process for the SwarmApp. It includes:

- **Build Stage**: Compiles the C++ application using CMake
- **Test Stage**: Runs unit tests using Google Test
- **Package Stage**: Builds and pushes Docker images
- **Deploy Stage**: Deploys to staging and production environments

## Pipeline Files

### `.gitlab-ci.yml` (Full Pipeline)
Complete CI/CD pipeline with:
- Build and test stages
- Docker image packaging
- Staging and production deployments
- Security scanning
- Manual deployment approval for production

### `.gitlab-ci-simple.yml` (Simple Pipeline)
Basic pipeline for development:
- Build and test stages only
- Optional Docker packaging
- No deployment stages

## Setup Instructions

### 1. GitLab Repository Setup

1. Push your code to a GitLab repository
2. Ensure the repository has the following structure:
   ```
   ├── .gitlab-ci.yml
   ├── CMakeLists.txt
   ├── docker/
   │   ├── Dockerfile
   │   ├── docker-compose.yml
   │   └── docker-compose.swarm.yml
   ├── src/
   ├── include/
   └── tests/
   ```

### 2. GitLab Variables

Set up the following variables in your GitLab project:

**Required Variables:**
- `CI_REGISTRY`: GitLab container registry URL (auto-set)
- `CI_REGISTRY_USER`: Registry username (auto-set)
- `CI_REGISTRY_PASSWORD`: Registry password (auto-set)

**Optional Variables:**
- `DOCKER_HOST`: Docker daemon host (for remote deployments)
- `DEPLOY_SSH_KEY`: SSH private key for deployment servers

### 3. Branch Strategy

The pipeline uses the following branch strategy:

- **`main`**: Production deployments (manual approval required)
- **`develop`**: Staging deployments (automatic)
- **`feature/*`**: Build and test only
- **Merge Requests**: Build and test only

## Pipeline Stages

### Build Stage
- Uses `gcc:11` image
- Installs CMake
- Compiles the application using CMake
- Creates build artifacts

### Test Stage
- Uses `gcc:11` image
- Installs Google Test
- Runs unit tests
- Generates JUnit test reports

### Package Stage
- Uses `docker:20.10.16` image
- Builds Docker image
- Pushes to GitLab container registry
- Tags with commit SHA and `latest`

### Deploy Stage

#### Staging Deployment
- Automatic deployment on `develop` branch
- Uses Docker Swarm
- Health checks included

#### Production Deployment
- Manual approval required
- Deploys on `main` branch or tags
- Uses Docker Swarm
- Includes rollback capability

## Deployment Script

The `scripts/deploy.sh` script provides additional deployment functionality:

```bash
# Deploy to production
./scripts/deploy.sh production swarm-app

# Deploy to staging
./scripts/deploy.sh staging swarm-app-staging

# Check deployment status
./scripts/deploy.sh status

# Perform health check
./scripts/deploy.sh health

# Rollback deployment
./scripts/deploy.sh rollback
```

## Environment Setup

### Staging Environment
- URL: `https://staging.swarmapp.example.com`
- Automatic deployment from `develop` branch
- Used for testing and validation

### Production Environment
- URL: `https://swarmapp.example.com`
- Manual deployment from `main` branch
- High availability with Docker Swarm

## Monitoring and Logs

### Pipeline Logs
- View pipeline execution in GitLab CI/CD interface
- Download build artifacts
- View test reports

### Application Logs
- Docker service logs: `docker service logs swarm-app_web`
- Stack status: `docker stack ps swarm-app`
- Service status: `docker stack services swarm-app`

## Troubleshooting

### Common Issues

1. **Build Failures**
   - Check CMake configuration
   - Verify all dependencies are installed
   - Review build logs for specific errors

2. **Test Failures**
   - Ensure Google Test is properly installed
   - Check test code for issues
   - Review test output for specific failures

3. **Deployment Failures**
   - Verify Docker Swarm is initialized
   - Check network connectivity
   - Review deployment logs

4. **Registry Issues**
   - Verify GitLab registry credentials
   - Check image tags and names
   - Ensure registry permissions

### Debug Commands

```bash
# Check Docker Swarm status
docker info | grep Swarm

# List running stacks
docker stack ls

# Check service status
docker stack services swarm-app

# View service logs
docker service logs swarm-app_web

# Check network
docker network ls | grep swarm
```

## Security Considerations

1. **Secrets Management**
   - Use GitLab CI/CD variables for sensitive data
   - Never commit secrets to the repository
   - Use environment-specific configurations

2. **Image Security**
   - Regular security scans (optional stage)
   - Use minimal base images
   - Keep dependencies updated

3. **Access Control**
   - Limit production deployment permissions
   - Use protected branches
   - Implement approval workflows

## Customization

### Adding New Stages
1. Add stage name to `stages` list
2. Create job configuration
3. Define dependencies and artifacts

### Environment-Specific Configurations
1. Create environment-specific compose files
2. Use GitLab variables for configuration
3. Implement environment-specific deployment scripts

### Integration with Other Tools
- Add notification stages (Slack, email)
- Integrate with monitoring tools
- Add performance testing stages

## Best Practices

1. **Pipeline Design**
   - Keep stages focused and fast
   - Use caching for dependencies
   - Implement proper error handling

2. **Testing**
   - Run tests in parallel when possible
   - Include integration tests
   - Maintain good test coverage

3. **Deployment**
   - Use blue-green or rolling deployments
   - Implement health checks
   - Have rollback procedures ready

4. **Monitoring**
   - Set up application monitoring
   - Monitor pipeline performance
   - Track deployment metrics
