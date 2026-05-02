#!/bin/bash

trap "kill 0" EXIT

(cd backend && php artisan serve) &
(cd frontend && npm run start)

wait
