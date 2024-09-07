withCredentials([string(credentialsId: 'PIPELINES_REPO', variable: 'checkout_url')]) {
    library(
        identifier: 'origin@master', 
        retriever: modernSCM(
            [
                $class: 'GitSCMSource',
                remote: checkout_url,
                credentialsId: 'CHECKOUT_TOKEN'
            ]
        )
    )
}

pipeline_github_igfd()
