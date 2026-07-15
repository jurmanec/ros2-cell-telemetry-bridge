'use strict';

/**
 * Serverless Framework v3's bundled awsLambdaRuntime enum stops at python3.11.
 * AWS Lambda accepts python3.12; this patches the schema so validation matches AWS.
 */
class AwsLambdaRuntimeSchemaPatch {
  constructor(serverless) {
    const def = serverless.configSchemaHandler.schema.definitions.awsLambdaRuntime;
    if (!def || !Array.isArray(def.enum)) {
      return;
    }
    if (!def.enum.includes('python3.12')) {
      def.enum.push('python3.12');
    }
  }
}

module.exports = AwsLambdaRuntimeSchemaPatch;
