
hello:
	@echo
	@echo Hello! run "make create" or "make destroy" or something.
	@echo

create:
	kubectl apply -f monitoring/prometheus.yaml
	kubectl apply -f monitoring/grafana.yaml
	kubectl apply -f monitoring/cluster_role.yaml

destroy:
	kubectl delete -f monitoring/prometheus.yaml
	kubectl delete -f monitoring/grafana.yaml
	kubectl delete -f monitoring/cluster_role.yaml